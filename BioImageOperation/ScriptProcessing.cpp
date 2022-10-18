/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <filesystem>
#include "KeepAlive.h"
#include "ScriptProcessing.h"
#include "ImageOperations.h"
#include "NumericPath.h"
#include "TextObserver.h"
#include "Constants.h"
#include "Util.h"


ScriptProcessing::ScriptProcessing() {
	// initialise static lookup tables
	ColorScale::init();
}

ScriptProcessing::~ScriptProcessing() {
	if (imageList) {
		delete imageList;
		imageList = nullptr;
	}

	if (imageSeries) {
		delete imageSeries;
		imageSeries = nullptr;
	}

	if (accumBuffer) {
		delete accumBuffer;
		accumBuffer = nullptr;
	}

	if (backgroundBuffer) {
		delete backgroundBuffer;
		backgroundBuffer = nullptr;
	}

	if (simpleBuffer) {
		delete simpleBuffer;
		simpleBuffer = nullptr;
	}

	if (imageTrackers) {
		delete imageTrackers;
		imageTrackers = nullptr;
	}

	if (scriptOperations) {
		delete scriptOperations;
		scriptOperations = nullptr;
	}

	if (dummyImage) {
		delete dummyImage;
		dummyImage = nullptr;
	}
}

void ScriptProcessing::reset() {
	basepath = "";
	sourceFile = "";
	sourceFilei = 0;
	nsourceFiles = 0;
	sourceWidth = 0;
	sourceHeight = 0;
	sourceFps = 0;
	sourceFrames = 0;
	sourceFrameNumber = 0;
	pixelSize = 1;
	windowSize = 1;
	logPower = 0;
	logPalette = Palette::Grayscale;
	operationMode = OperationMode::Idle;

	scriptOperations->reset();
	imageList->reset();
	backgroundBuffer->reset();
	simpleBuffer->reset();
	imageSeries->reset();
	accumBuffer->reset();

	imageTrackers->reset();
}

void ScriptProcessing::registerObserver(Observer* observer) {
	this->observer = observer;
}

bool ScriptProcessing::startProcessNoGui(string scriptFilename) {
	TextObserver observer;
	string script;

	registerObserver(&observer);
	useGui = false;

	reset();
	if (!filesystem::exists(scriptFilename)) {
		showDialog("Script file not found: " + scriptFilename, MessageLevel::Error);
		return false;
    }

	basepath = Util::extractFilePath(scriptFilename);
	if (basepath == "") {
		basepath = filesystem::current_path().string();
	}

	try {
		script = Util::readText(scriptFilename);
		scriptOperations->extract(script);
		this->observer->resetProgressTimer();
		operationMode = OperationMode::Run;
		processThreadMethod();
	} catch (exception& e) {
		showDialog(Util::getExceptionDetail(e), MessageLevel::Error);
		doReset();
		return false;
	}
	return true;
}

bool ScriptProcessing::startProcess(string filepath, string script) {
	try {
		if (operationMode != OperationMode::Run) {
			if (operationMode == OperationMode::Idle) {
                reset();
                basepath = Util::extractFilePath(filepath);
				scriptOperations->extract(script);
			}
			observer->resetProgressTimer();
			operationMode = OperationMode::Run;
			processThread = new std::thread(&ScriptProcessing::processThreadMethod, this);
		} else {	// operationMode == Run
			operationMode = OperationMode::RequestPause;
		}
		setMode(operationMode);
	} catch (exception& e) {
		showDialog(Util::getExceptionDetail(e), MessageLevel::Error);
		doReset();
		return false;
	}
	return true;
}

void ScriptProcessing::processThreadMethod() {
	KeepAlive::startKeepAlive();
	processOperations(scriptOperations, nullptr);
	KeepAlive::stopKeepAlive();
	if (operationMode == OperationMode::RequestPause) {
		setMode(OperationMode::Pause);
	}
	if (operationMode != OperationMode::Pause) {
		this_thread::sleep_for(100ms);		// finish async tasks (show image)
		doReset();
	}
}

void ScriptProcessing::processOperations(ScriptOperations* operations, ScriptOperation* prevOperation0) {
	bool isRoot = (prevOperation0 == nullptr);
	ScriptOperation* operation;
	ScriptOperation* prevOperation = prevOperation0;
	bool operationFinished;

	while (operationMode == OperationMode::Run || operationMode == OperationMode::RequestPause) {
		operation = operations->getCurrentOperation();
		if (operation) {
			operation->reset();
			if (!prevOperation0) {
				showOperations(operations, operation);
			}
			operationFinished = processOperation(operation, prevOperation);
			operation->finish();
			if (operationFinished) {
				operations->moveNextOperation();
			}
			if (operationMode == OperationMode::RequestPause && (isRoot || operation->hasInnerOperations())) {
				break;
			}
			prevOperation = operation;
		} else {
			break;
		}
	}
}

bool ScriptProcessing::processOperation(ScriptOperation* operation, ScriptOperation* prevOperation) {
	int count = operation->count;
    string errorMsg;

	operation->count++;

	if (operation->interval > 1) {
		if ((count % operation->interval) != operation->offset) {
			return true;
		}
	}

	Mat* image = nullptr;		// pointer to source image
	Mat* newImage = nullptr;	// pointer to new image
	Mat* refImage = nullptr;	// auxiliary image pointer
	bool newImageSet = false;	// if newImage is set

	if (prevOperation) {
		image = prevOperation->imageRef;				// source: pointer to previous operation image
		operation->imageRef = prevOperation->imageRef;	// default: set current operation image to previous operation image
	}

	if (!image) {
		image = dummyImage;								// prevent null error
	}

	newImage = &operation->image;						// newImage is pointer to current operation image

	NumericPath sourcePath, outputPath;
	ImageTracker* imageTracker;
	string path, source, output, label;
	int width, height;
	int displayi;
	double fps, size;
	int frame = sourceFrameNumber;

	int delay;
	bool debugMode;
	bool done = true;

	try {
		switch (operation->operationType) {
		case ScriptOperationType::Set:
			path = operation->getArgument(ArgumentLabel::Path);
			if (path != "") {
				basepath = path;
			}
			width = (int)operation->getArgumentNumeric(ArgumentLabel::Width);
			if (width != 0) {
				sourceWidth = width;
			}
			height = (int)operation->getArgumentNumeric(ArgumentLabel::Height);
			if (height != 0) {
				sourceHeight = height;
			}
			fps = operation->getArgumentNumeric(ArgumentLabel::Fps);
			if (fps != 0) {
				sourceFps = fps;
			}
			size = operation->getArgumentNumeric(ArgumentLabel::PixelSize);
			if (size != 0) {
				pixelSize = size;
			}
			size = operation->getArgumentNumeric(ArgumentLabel::WindowSize);
			if (size != 0) {
				windowSize = size;
			}
			break;

		case ScriptOperationType::SetPath:
			basepath = operation->getArgument(ArgumentLabel::Path);
			break;

		case ScriptOperationType::Source:
			imageTrackers->reset();
			observer->resetProgressTimer();
			sourcePath.setInputPath(basepath, operation->getArgument(ArgumentLabel::Path));
			sourceFile = sourcePath.createFilePath(sourceFilei);
			if (sourceFile != "") {
				nsourceFiles = sourcePath.totaln;
				done = false;
				sourceFilei++;
			} else {
				// already past last file (current invalid)
				return true;
			}
			break;

		case ScriptOperationType::CreateImage:
			width = (int)operation->getArgumentNumeric(ArgumentLabel::Width);
			height = (int)operation->getArgumentNumeric(ArgumentLabel::Height);
			if (width == 0 && height == 0) {
				width = sourceWidth;
				height = sourceHeight;
			}
			ImageOperations::create(newImage, width, height,
									(ImageColorMode)operation->getArgument(ArgumentLabel::ColorMode,
									(int)ImageColorMode::Color),
									operation->getArgumentNumeric(ArgumentLabel::Red),
									operation->getArgumentNumeric(ArgumentLabel::Green),
									operation->getArgumentNumeric(ArgumentLabel::Blue));
			sourceWidth = width;
			sourceHeight = height;
			sourceFrameNumber = 0;
			newImageSet = true;
			break;

		case ScriptOperationType::OpenImage:
			if (sourceFile != "") {
				source = sourceFile;
			} else {
				source = operation->getArgument(ArgumentLabel::Path);
			}
			operation->initFrameSource(FrameType::Image, 0, basepath, source,
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length),
										sourceFps,
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval),
										(int)operation->getArgumentNumeric(ArgumentLabel::Total));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				sourceFrames = operation->frameSource->getTotalFrames();
				showStatus(operation->frameSource->getCurrentFrame(), sourceFrames);
				done = false;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			newImageSet = true;
			if (done) {
				operation->resetFrameSource();
			}
			break;

        case ScriptOperationType::OpenVideo:
			if (sourceFile != "") {
				source = sourceFile;
			} else {
				source = operation->getArgument(ArgumentLabel::Path);
			}
			operation->initFrameSource(FrameType::Video,
										(int)operation->getArgumentNumeric(ArgumentLabel::API), basepath, source,
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length), 0,
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval),
										(int)operation->getArgumentNumeric(ArgumentLabel::Total));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				label = getSourceLabel() + operation->frameSource->getLabel();
				showStatus(operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), label);
				done = false;
			} else {
				// already past last frame; current image invalid
				operation->resetFrameSource();
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			sourceFps = operation->frameSource->getFps();
			sourceFrames = operation->frameSource->getTotalFrames();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenCapture:
			source = operation->getArgument(ArgumentLabel::Path);
			if (source == "") {
				source = to_string((int)operation->getArgumentNumeric());
			}
			operation->initFrameSource(FrameType::Capture,
										(int)operation->getArgumentNumeric(ArgumentLabel::API), basepath, source,
										"", operation->getArgument(ArgumentLabel::Length),
										operation->getArgumentNumeric(ArgumentLabel::Fps),
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval),
										(int)operation->getArgumentNumeric(ArgumentLabel::Total));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				showStatus(operation->frameSource->getCurrentFrame());
				done = false;
			} else {
				// capture failed; current image invalid
				operation->resetFrameSource();
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			sourceFrames = 0;
			newImageSet = true;
			break;

		case ScriptOperationType::SaveImage:
			operation->initFrameOutput(FrameType::Image, basepath,
										operation->getArgument(ArgumentLabel::Path), Constants::defaultImageExtension,
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length), sourceFps);
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image));
			break;

		case ScriptOperationType::SaveVideo:
			fps = operation->getArgumentNumeric(ArgumentLabel::Fps);
			if (fps == 0) {
				fps = sourceFps;
			}
			operation->initFrameOutput(FrameType::Video, basepath,
										operation->getArgument(ArgumentLabel::Path), Constants::defaultVideoExtension,
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length), fps,
										operation->getArgument(ArgumentLabel::Codec));
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image));
			break;

		case ScriptOperationType::ShowImage:
			refImage = getLabelOrCurrentImage(operation, image);
			if (Util::isValidImage(refImage)) {
				showImage(refImage,
							(int)operation->getArgumentNumeric(ArgumentLabel::None, true),
							"#" + to_string(count));
			}
			break;

		case ScriptOperationType::GetImage:
			*newImage = *imageList->getImage(operation->getArgument());
			newImageSet = true;
			break;

		case ScriptOperationType::StoreImage:
			imageList->setImage(image, operation->getArgument());
			break;

		case ScriptOperationType::Scale:
			ImageOperations::scale(*getLabelOrCurrentImage(operation, image), *newImage,
									operation->getArgumentNumeric(ArgumentLabel::Width),
									operation->getArgumentNumeric(ArgumentLabel::Height));
			sourceWidth = newImage->cols;
			sourceHeight = newImage->rows;
			newImageSet = true;
			break;

		case ScriptOperationType::Crop:
			ImageOperations::crop(*getLabelOrCurrentImage(operation, image), newImage,
									operation->getArgumentNumeric(ArgumentLabel::Width),
									operation->getArgumentNumeric(ArgumentLabel::Height),
									operation->getArgumentNumeric(ArgumentLabel::X),
									operation->getArgumentNumeric(ArgumentLabel::Y));
			sourceWidth = newImage->cols;
			sourceHeight = newImage->rows;
			newImageSet = true;
			break;

		case ScriptOperationType::Mask:
			ImageOperations::mask(*image, *imageList->getImage(operation->getArgument()), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Grayscale:
			ImageOperations::convertToGrayScale(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Color:
			ImageOperations::convertToColor(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ColorAlpha:
			ImageOperations::convertToColorAlpha(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Int:
			ImageOperations::convertToInt(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Float:
			ImageOperations::convertToFloat(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetSaturation:
			ImageOperations::getSaturation(*getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsValue:
			ImageOperations::getHsValue(*getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsLightness:
			ImageOperations::getHsLightness(*getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Threshold:
			ImageOperations::threshold(*getLabelOrCurrentImage(operation, image), *newImage, operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::Erode:
			ImageOperations::erode(*getLabelOrCurrentImage(operation, image), *newImage, (int)operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::Dilate:
			ImageOperations::dilate(*getLabelOrCurrentImage(operation, image), *newImage, (int)operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::Difference:
			ImageOperations::difference(*image, *imageList->getImage(operation->getArgument()), *newImage, false);
			newImageSet = true;
			break;

		case ScriptOperationType::DifferenceAbs:
			ImageOperations::difference(*image, *imageList->getImage(operation->getArgument()), *newImage, true);
			newImageSet = true;
			break;

		case ScriptOperationType::Add:
			ImageOperations::add(*image, *imageList->getImage(operation->getArgument()), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Multiply:
			ImageOperations::multiply(*getLabelOrCurrentImage(operation, image), operation->getArgumentNumeric(), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Invert:
			ImageOperations::invert(*getLabelOrCurrentImage(operation, image), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::SetBackground:
			backgroundBuffer->setImage(getLabelOrCurrentImage(operation, image));
			break;

		case ScriptOperationType::UpdateBackground:
			backgroundBuffer->addWeighted(getLabelOrCurrentImage(operation, image), newImage, operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateWeight:
			simpleBuffer->addWeighted(getLabelOrCurrentImage(operation, image), newImage, operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateMin:
			simpleBuffer->addMin(getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateMax:
			simpleBuffer->addMax(getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ClearSeries:
			imageSeries->reset();
			break;

		case ScriptOperationType::AddSeries:
			imageSeries->addImage(getLabelOrCurrentImage(operation, image), (int)operation->getArgumentNumeric());
			break;

		case ScriptOperationType::GetSeriesMedian:
			medianMode = (MedianMode)operation->getArgument(ArgumentLabel::MedianMode, (int)MedianMode::Normal);
			newImageSet = imageSeries->getMedian(*newImage, medianMode);
			break;

		case ScriptOperationType::GetSeriesMean:
			newImageSet = imageSeries->getMean(*newImage);
			break;

		case ScriptOperationType::AddAccum:
			accumBuffer->addImage(getLabelOrCurrentImage(operation, image), (AccumMode)operation->getArgument(ArgumentLabel::AccumMode, (int)AccumMode::Age));
			break;

		case ScriptOperationType::GetAccum:
			logPower = operation->getArgumentNumeric();
			logPalette = (Palette)operation->getArgument(ArgumentLabel::Palette, (int)Palette::Grayscale);
			accumBuffer->getImage(newImage, (float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::CreateClusters:
			debugMode = operation->getArgumentBoolean(ArgumentLabel::Debug);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker),
												TrackingMethod::Any,
												sourceFps, pixelSize, windowSize, observer);
			output = imageTracker->createClusters(image, operation->getArgumentNumeric(ArgumentLabel::MinArea),
													operation->getArgumentNumeric(ArgumentLabel::MaxArea),
													sourceFrames, basepath, debugMode);
			if (debugMode) {
				showText(output, Constants::nTextWindows);
			}
			break;

		case ScriptOperationType::OpticalCalibration:
			debugMode = operation->getArgumentBoolean(ArgumentLabel::Debug);
			if (!opticalCorrection->calibrate(*getLabelOrCurrentImage(operation, image),
											operation->getArgumentNumeric(ArgumentLabel::NX),
											operation->getArgumentNumeric(ArgumentLabel::NY),
											debugMode, *newImage)) {
				showDialog("Optical calibration failed based on consistent internal edges", MessageLevel::Error);
			} else if (debugMode) {
				newImageSet = true;
			}
			break;

		case ScriptOperationType::OpticalCorrection:
			if (!opticalCorrection->undistort(*getLabelOrCurrentImage(operation, image), *newImage)) {
				showDialog("Optical correction not calibrated", MessageLevel::Error);
			}
			newImageSet = true;
			break;

		case ScriptOperationType::CreateTracks:
			debugMode = operation->getArgumentBoolean(ArgumentLabel::Debug);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			output = imageTracker->createTracks(operation->getArgumentNumeric(ArgumentLabel::MaxMove),
												(int)operation->getArgumentNumeric(ArgumentLabel::MinActive),
												(int)operation->getArgumentNumeric(ArgumentLabel::MaxInactive),
												sourceFrames, basepath, debugMode);
			if (debugMode) {
				showText(output, Constants::nTextWindows);
			}
			break;

		case ScriptOperationType::CreatePaths:
			debugMode = operation->getArgumentBoolean(ArgumentLabel::Debug);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			output = imageTracker->createPaths(operation->getArgumentNumeric(ArgumentLabel::Distance), debugMode);
			if (debugMode) {
				showText(output, Constants::nTextWindows);
			}
			break;

		case ScriptOperationType::DrawClusters:
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawClusters(getLabelOrCurrentImage(operation, image), newImage,
										operation->getArgument(ArgumentLabel::DrawMode, (int)ClusterDrawMode::ClusterDefault));
			newImageSet = true;
			break;

		case ScriptOperationType::DrawTracks:
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawTracks(getLabelOrCurrentImage(operation, image), newImage,
										operation->getArgument(ArgumentLabel::DrawMode, (int)ClusterDrawMode::TracksDefault),
										(int)sourceFps);
			newImageSet = true;
			break;

		case ScriptOperationType::DrawPaths:
			logPower = operation->getArgumentNumeric();
			logPalette = (Palette)operation->getArgument(ArgumentLabel::Palette, (int)Palette::Grayscale);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawPaths(getLabelOrCurrentImage(operation, image), newImage,
									(PathDrawMode)operation->getArgument(ArgumentLabel::PathDrawMode, (int)PathDrawMode::Age),
									(float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::DrawTrackCount:
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawTrackCount(getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ShowTrackInfo:
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			showText(imageTracker->getInfo(),
					(int)operation->getArgumentNumeric(ArgumentLabel::Display, true),
					"#" + to_string(count));
			break;

		case ScriptOperationType::SaveClusters:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), sourceFile, Constants::defaultDataExtension);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->saveClusters(outputPath.createFilePath(frame), frame, getTime(frame),
										(SaveFormat)operation->getArgument(ArgumentLabel::Format, (int)SaveFormat::ByTime),
										operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SaveTracks:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), sourceFile, Constants::defaultDataExtension);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->saveTracks(outputPath.createFilePath(frame), frame, getTime(frame),
										(SaveFormat)operation->getArgument(ArgumentLabel::Format, (int)SaveFormat::ByTime),
										operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SavePaths:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), sourceFile, Constants::defaultDataExtension);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->savePaths(outputPath.createFilePath(frame), frame, getTime(frame));
			break;

		case ScriptOperationType::SaveTrackInfo:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), sourceFile, Constants::defaultDataExtension);
			imageTracker = imageTrackers->get(operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->saveTrackInfo(outputPath.createFilePath(frame), frame, getTime(frame));
			break;

		case ScriptOperationType::DrawLegend:
			displayi = (int)operation->getArgumentNumeric(ArgumentLabel::Display);
			if (displayi > 0) {
				// show in separate display
				ImageOperations::create(newImage, 1000, 1000);
				ImageOperations::drawLegend(*newImage, *newImage, DrawPosition::Full, logPower, logPalette);
				showImage(newImage, displayi - 1);		// manual one-base correction
			} else {
				// draw on image
				ImageOperations::drawLegend(*getLabelOrCurrentImage(operation, image), *newImage,
											(DrawPosition)operation->getArgument(ArgumentLabel::Position, (int)DrawPosition::BottomRight),
											logPower, logPalette);
				newImageSet = true;
			}
			break;

		case ScriptOperationType::Wait:
			delay = (int)operation->getArgumentNumeric();
			if (delay == 0) {
				delay = 1000;
			}
			this_thread::sleep_for(chrono::milliseconds(delay));
			break;

		case ScriptOperationType::Pause:
			requestPause();
			break;

		case ScriptOperationType::Benchmark:
			scriptOperations->updateBenchmarking();
			if (!useGui) {
				output = scriptOperations->renderOperations();
				showText(output, Constants::nTextWindows);
			}
			break;

			// end of switch
		}

		if (operation->hasInnerOperations()) {
			if (newImageSet) {
				operation->imageRef = newImage;
			}
			operation->initialFinish();
			processOperations(operation->innerOperations, operation);
		}

		if (operation->asignee != "") {
			if (newImageSet) {
				imageList->setImage(newImage, operation->asignee);
			} else {
				imageList->setImage(image, operation->asignee);
			}
		} else {
			if (newImageSet) {
				operation->imageRef = newImage;
			}
		}
    } catch (cv::Exception& e) {
		errorMsg = e.what();
		if (Util::contains(errorMsg, "==")) {
			// adding more user friendly messages:
			if (Util::contains(errorMsg, "CV_MAT_TYPE")) {
				errorMsg += " (Image types don't match)";
			}
			if (Util::contains(errorMsg, "CV_MAT_CN")) {
				errorMsg += " (Image color channels don't match)";
			}
			if (Util::contains(errorMsg, "channels()")) {
				errorMsg += " (Unexpected number of color channels)";
			}
			if (Util::contains(errorMsg, "depth()")) {
				errorMsg += " (Unexpected image depth/type)";
			}
			if (Util::contains(errorMsg, "size()")) {
				errorMsg += " (Unexpected image size)";
			}
		} else if (Util::contains(errorMsg, "different types")) {
			errorMsg += " (Image depth/types don't match)";
		}
		errorMsg += " in\n" + operation->line;
		showDialog(errorMsg, MessageLevel::Error);
		doReset();
    } catch (std::exception& e) {
		errorMsg = Util::getExceptionDetail(e) + " in\n" + operation->line;
		showDialog(errorMsg, MessageLevel::Error);
		doReset();
	}
	return done;
}

Mat* ScriptProcessing::getLabelOrCurrentImage(ScriptOperation* operation, Mat* currentImage) {
	Mat* image;
	string label = operation->getArgument(ArgumentLabel::Label);
	image = imageList->getImage(label, false);
	if (!Util::isValidImage(image)) {
		image = currentImage;
	}
	return image;
}

OperationMode ScriptProcessing::getMode() {
	return operationMode;
}

double ScriptProcessing::getTime(int frame) {
	double time;
	if (sourceFps > 0) {
		time = frame / sourceFps;
	} else {
		time = frame;
	}
	return time;
}

string ScriptProcessing::getSourceLabel() {
	string label = "";
	if (nsourceFiles > 0) {
		label = Util::format("%.0f%% ", (double)(sourceFilei - 1) / nsourceFiles * 100);
	}
	return label;
}

void ScriptProcessing::requestPause() {
	setMode(OperationMode::RequestPause);
}

void ScriptProcessing::requestAbort() {
	if (operationMode == OperationMode::Pause) {
		doReset();
	} else {
		setMode(OperationMode::Abort);
	}
}

void ScriptProcessing::doReset() {
	imageTrackers->close();
	scriptOperations->close();

	observer->resetProgressTimer();
	setMode(OperationMode::Idle);
	clearStatus();
}

void ScriptProcessing::setMode(OperationMode mode) {
	operationMode = mode;
	observer->setMode((int)operationMode);
}

void ScriptProcessing::clearStatus() {
	observer->clearStatus();
}

void ScriptProcessing::showStatus(int i, int tot, string label) {
	if (observer->checkStatusProcess()) {
		observer->showStatus(i, tot, label);
	}
}

void ScriptProcessing::showText(string text, int displayi, string reference) {
	if (observer->checkTextProcess(displayi)) {
		observer->showText(text, displayi, reference);
	}
}

void ScriptProcessing::showImage(Mat* image, int displayi, string reference) {
	if (observer->checkImageProcess(displayi)) {
		observer->showImage(image, displayi, reference);
	}
}

void ScriptProcessing::showDialog(string message, MessageLevel level) {
	cout << "\n" + MessageLevels[(int)level] + " " + message << endl;
	observer->showDialog(message, (int)level);
}

void ScriptProcessing::showOperations(ScriptOperations* operations, ScriptOperation* currentOperation) {
	if (observer->checkOperationsProcess()) {
		observer->showOperations(operations, currentOperation);
	}
}
