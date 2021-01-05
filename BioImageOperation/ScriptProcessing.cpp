/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <filesystem>
#include "ScriptProcessing.h"
#include "MainWindow.h"
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
		imageList = NULL;
	}

	if (imageSeries) {
		delete imageSeries;
		imageSeries = NULL;
	}

	if (accumBuffer) {
		delete accumBuffer;
		accumBuffer = NULL;
	}

	if (backgroundBuffer) {
		delete backgroundBuffer;
		backgroundBuffer = NULL;
	}

	if (averageBuffer) {
		delete averageBuffer;
		averageBuffer = NULL;
	}

	if (imageTrackers) {
		delete imageTrackers;
		imageTrackers = NULL;
	}

	if (scriptOperations) {
		delete scriptOperations;
		scriptOperations = NULL;
	}

	if (dummyImage) {
		delete dummyImage;
		dummyImage = NULL;
	}
}

void ScriptProcessing::reset() {
	basepath = "";
	sourceWidth = 0;
	sourceHeight = 0;
	sourceFps = 0;
	sourceFrameNumber = 0;
	logPower = 0;
	operationMode = OperationMode::Idle;

	scriptOperations->reset();
	imageList->reset();
	backgroundBuffer->reset();
	averageBuffer->reset();
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

	this->observer = &observer;

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
		scriptOperations->extract(script, 0);
		this->observer->resetProgressTimer();
		operationMode = OperationMode::Run;
		processThreadMethod();
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), MessageLevel::Error);
		doAbort();
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
				scriptOperations->extract(script, 0);
			}
			observer->resetProgressTimer();
			operationMode = OperationMode::Run;
			processThread = new std::thread(&ScriptProcessing::processThreadMethod, this);
		} else {
			operationMode = OperationMode::Pause;
		}
		setMode(operationMode);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), MessageLevel::Error);
		doAbort();
		return false;
	}
	return true;
}

void ScriptProcessing::processThreadMethod() {
	processOperations(scriptOperations, NULL);
	if (operationMode != OperationMode::Pause) {
		doAbort();
	}
}

void ScriptProcessing::processOperations(ScriptOperations* operations, ScriptOperation* prevOperation0) {
	ScriptOperation* operation;
	ScriptOperation* prevOperation = prevOperation0;
	bool operationFinished;

	while (operationMode == OperationMode::Run) {
		operation = operations->getCurrentOperation();
		if (operation) {
			operation->reset();
			operationFinished = processOperation(operation, prevOperation);
			if (debugMode) {
				operation->finish();
			}
			if (operationFinished) {
				operations->moveNextOperation();
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

	Mat* image = NULL;			// pointer to source image
	Mat* newImage = NULL;		// pointer to new image
	Mat* refImage = NULL;		// auxiliary image pointer
	bool newImageSet = false;	// if newImage is set

	if (prevOperation) {
		image = prevOperation->imageRef;				// source: pointer to previous operation image
		operation->imageRef = prevOperation->imageRef;	// default: set current operation image to previous operation image
	}

	if (!image) {
		image = dummyImage;								// prevent NULL error
	}

	newImage = &operation->image;						// newImage is pointer to current operation image

	NumericPath outputPath;
	ImageTracker* imageTracker;
	string source, trackerId, output;
	int width, height;
	int displayi;
	double fps;
	int frame = sourceFrameNumber;

	int delay;
	bool done = true;

	try {
		switch (operation->operationType) {
		case ScriptOperationType::SetPath:
			basepath = operation->getArgument(ArgumentLabel::Path);
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
			operation->initFrameSource(FrameType::Image, 0, basepath,
										operation->getArgument(ArgumentLabel::Path),
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length),
										sourceFps,
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				showStatus(operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames());
				done = false;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenVideo:
			operation->initFrameSource(FrameType::Video,
										(int)operation->getArgumentNumeric(ArgumentLabel::API), basepath,
										operation->getArgument(ArgumentLabel::Path),
										operation->getArgument(ArgumentLabel::Start),
										operation->getArgument(ArgumentLabel::Length), 0,
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				showStatus(operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), operation->frameSource->getLabel());
				done = false;
			} else {
				// already past last frame; current image invalid
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			sourceFps = operation->frameSource->getFps();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenCapture:
			source = operation->getArgument(ArgumentLabel::Path);
			if (source != "") {
				source = to_string(operation->getArgumentNumeric());
			}
			operation->initFrameSource(FrameType::Capture,
										(int)operation->getArgumentNumeric(ArgumentLabel::API),
										basepath, source, "", "", 0,
										(int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				showStatus(operation->frameSource->getCurrentFrame());
				done = false;
			} else {
				// capture failed; current image invalid
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
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
			newImageSet = true;
			break;

		case ScriptOperationType::Crop:
			ImageOperations::crop(getLabelOrCurrentImage(operation, image), newImage,
									operation->getArgumentNumeric(ArgumentLabel::Width),
									operation->getArgumentNumeric(ArgumentLabel::Height),
									operation->getArgumentNumeric(ArgumentLabel::X),
									operation->getArgumentNumeric(ArgumentLabel::Y));
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

		case ScriptOperationType::UpdateBackground:
			backgroundBuffer->addImage(getLabelOrCurrentImage(operation, image), operation->getArgumentNumeric());
			backgroundBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateAverage:
			averageBuffer->addImage(getLabelOrCurrentImage(operation, image), operation->getArgumentNumeric());
			averageBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ClearSeries:
			imageSeries->reset();
			break;

		case ScriptOperationType::AddSeries:
			imageSeries->addImage(getLabelOrCurrentImage(operation, image), (int)operation->getArgumentNumeric());
			break;

		case ScriptOperationType::GetSeriesMedian:
			newImageSet = imageSeries->getMedian(*newImage);
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
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker), true);
			imageTracker->createClusters(image, operation->getArgumentNumeric(ArgumentLabel::MinArea),
										operation->getArgumentNumeric(ArgumentLabel::MaxArea),
										basepath, debugMode);
			break;

		case ScriptOperationType::CreateTracks:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->createTracks(operation->getArgumentNumeric(ArgumentLabel::MaxMove),
										(int)operation->getArgumentNumeric(ArgumentLabel::MinActive),
										(int)operation->getArgumentNumeric(ArgumentLabel::MaxInactive),
										basepath);
			break;

		case ScriptOperationType::CreatePaths:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->createPaths(operation->getArgumentNumeric(ArgumentLabel::Distance));
			break;

		case ScriptOperationType::DrawClusters:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawClusters(getLabelOrCurrentImage(operation, image), newImage,
										operation->getArgument(ArgumentLabel::DrawMode, (int)ClusterDrawMode::ClusterDefault));
			newImageSet = true;
			break;

		case ScriptOperationType::DrawTracks:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawTracks(getLabelOrCurrentImage(operation, image), newImage,
										operation->getArgument(ArgumentLabel::DrawMode, (int)ClusterDrawMode::TracksDefault),
										(int)sourceFps);
			newImageSet = true;
			break;

		case ScriptOperationType::DrawPaths:
			logPower = operation->getArgumentNumeric();
			logPalette = (Palette)operation->getArgument(ArgumentLabel::Palette, (int)Palette::Grayscale);
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawPaths(getLabelOrCurrentImage(operation, image), newImage,
									(PathDrawMode)operation->getArgument(ArgumentLabel::PathDrawMode, (int)PathDrawMode::Age),
									(float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::ShowTrackInfo:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			showText(imageTracker->getInfo(),
					(int)operation->getArgumentNumeric(ArgumentLabel::Display, true),
					"#" + to_string(count));
			break;

		case ScriptOperationType::DrawTrackInfo:
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->drawTrackInfo(getLabelOrCurrentImage(operation, image), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::SaveClusters:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->saveClusters(outputPath.createFilePath(frame), frame, getTime(frame),
										(SaveFormat)operation->getArgument(ArgumentLabel::Format, (int)SaveFormat::ByTime),
										operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SaveTracks:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->saveTracks(outputPath.createFilePath(frame), frame, getTime(frame),
										(SaveFormat)operation->getArgument(ArgumentLabel::Format, (int)SaveFormat::ByTime),
										operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SavePaths:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
			imageTracker->savePaths(outputPath.createFilePath(frame), frame, getTime(frame));
			break;

		case ScriptOperationType::SaveTrackInfo:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTracker = imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker));
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

		case ScriptOperationType::Debug:
			debugMode = true;
			trackerId = operation->getArgument(ArgumentLabel::Tracker);
			if (trackerId != "") {
				output = imageTrackers->getTracker(NULL, trackerId)->getDebugInfo();
			} else {
				output = scriptOperations->getDebug();
			}
			showText(output, Constants::nTextWindows);
			break;

			// end of switch
		}

		if (operation->hasInnerOperations()) {
			if (newImageSet) {
				operation->imageRef = newImage;
			}
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
	} catch (cv::Exception e) {
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
				errorMsg += " (Incorrect number of image color channels)";
			}
		}
		errorMsg += " in\n" + operation->line;
		showDialog(errorMsg, MessageLevel::Error);
		doAbort();
	} catch (std::exception e) {
		errorMsg = Util::getExceptionDetail(e) + " in\n" + operation->line;
		showDialog(errorMsg, MessageLevel::Error);
		doAbort();
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

double ScriptProcessing::getTime(int frame) {
	double time;
	if (sourceFps > 0) {
		time = frame / sourceFps;
	} else {
		time = frame;
	}
	return time;
}

void ScriptProcessing::doAbort() {
	operationMode = OperationMode::Idle;
	this_thread::sleep_for(100ms);

	imageTrackers->close();
	scriptOperations->close();

	observer->resetProgressTimer();
	setMode(OperationMode::Idle);
	clearStatus();
}

void ScriptProcessing::setMode(OperationMode mode) {
	observer->setMode((int)mode);
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
	observer->showDialog(message, (int)level);
}
