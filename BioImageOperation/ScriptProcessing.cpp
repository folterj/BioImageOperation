/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ScriptProcessing.h"
#include "MainWindow.h"
#include "ImageOperations.h"
#include "NumericPath.h"
#include "Constants.h"
#include "Util.h"


ScriptProcessing::ScriptProcessing() {
	// initialise static lookup tables
	ColorScale::init();

	//ScriptOperation::writeOperationList("D:\\BioImageOperation.rtf");
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
	abort = false;

	scriptOperations->reset();
	imageList->reset();
	backgroundBuffer->reset();
	averageBuffer->reset();
	imageSeries->reset();
	accumBuffer->reset();

	imageTrackers->reset();
	emit resetImages();
}

void ScriptProcessing::registerObserver(Observer* observer) {
	this->observer = observer;

	connect(this, &ScriptProcessing::resetUI, (MainWindow*)observer, &MainWindow::resetUI);
	connect(this, &ScriptProcessing::resetImages, (MainWindow*)observer, &MainWindow::resetImages);
	connect(this, &ScriptProcessing::clearStatus, (MainWindow*)observer, &MainWindow::clearStatus);
	connect(this, &ScriptProcessing::showStatus, (MainWindow*)observer, &MainWindow::showStatus);
	connect(this, &ScriptProcessing::showInfo, (MainWindow*)observer, &MainWindow::showInfo);
	connect(this, &ScriptProcessing::showImage, (MainWindow*)observer, &MainWindow::showImage);
	connect(this, &ScriptProcessing::showDialog, (MainWindow*)observer, &MainWindow::showDialog);
}

bool ScriptProcessing::startProcess(string filepath, string script) {
	reset();
	basepath = Util::extractFilePath(filepath);

	try {
		scriptOperations->extract(script, 0);
		observer->resetProgressTimer();

		//processThread = QThread::create(&ScriptProcessing::processThreadMethod, this);
		//processThread->start();
		processThread = new std::thread(&ScriptProcessing::processThreadMethod, this);
	} catch (exception e) {
		emit showDialog(Util::getExceptionDetail(e).c_str());
		doAbort();
		return false;
	}
	return true;
}

void ScriptProcessing::processThreadMethod() {
	processOperations(scriptOperations, NULL);
	doAbort();
}

void ScriptProcessing::processOperations(ScriptOperations* operations, ScriptOperation* prevOperation0) {
	ScriptOperation* operation;
	ScriptOperation* prevOperation = prevOperation0;
	bool operationFinished;

	while (!abort) {
		operation = operations->getCurrentOperation();
		if (operation) {
			operation->reset();
			operationFinished = processOperation(operation, prevOperation);
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
	string source;
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
			ImageOperations::create(newImage, width, height, (ImageColorMode)operation->getArgument(ArgumentLabel::ColorMode, ImageColorMode::Color), operation->getArgumentNumeric(ArgumentLabel::Red), operation->getArgumentNumeric(ArgumentLabel::Green), operation->getArgumentNumeric(ArgumentLabel::Blue));
			sourceWidth = width;
			sourceHeight = height;
			sourceFrameNumber = 0;
			newImageSet = true;
			break;

		case ScriptOperationType::OpenImage:
			operation->initFrameSource(FrameType::Image, 0, basepath, operation->getArgument(ArgumentLabel::Path), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), sourceFps, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				emit showStatus(operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames());
				done = false;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenVideo:
			operation->initFrameSource(FrameType::Video, (int)operation->getArgumentNumeric(ArgumentLabel::API), basepath, operation->getArgument(ArgumentLabel::Path), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), 0, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				emit showStatus(operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), operation->frameSource->getLabel().c_str());
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
			operation->initFrameSource(FrameType::Capture, (int)operation->getArgumentNumeric(ArgumentLabel::API), basepath, source, "", "", 0, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				emit showStatus(operation->frameSource->getCurrentFrame());
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
			operation->initFrameOutput(FrameType::Image, basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultImageExtension, operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), sourceFps);
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image, true));
			break;

		case ScriptOperationType::SaveVideo:
			fps = operation->getArgumentNumeric(ArgumentLabel::Fps);
			if (fps == 0) {
				fps = sourceFps;
			}
			operation->initFrameOutput(FrameType::Video, basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultVideoExtension, operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), fps, operation->getArgument(ArgumentLabel::Codec));
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image, true));
			break;

		case ScriptOperationType::ShowImage:
			refImage = getLabelOrCurrentImage(operation, image, false);
			if (Util::isValidImage(refImage)) {
				emit showImage(refImage, (int)operation->getArgumentNumeric(ArgumentLabel::None, true));
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
			ImageOperations::scale(*getLabelOrCurrentImage(operation, image, false), *newImage,
				(int)operation->getArgumentNumeric(ArgumentLabel::Width), (int)operation->getArgumentNumeric(ArgumentLabel::Height));
			newImageSet = true;
			break;

		case ScriptOperationType::Crop:
			ImageOperations::crop(getLabelOrCurrentImage(operation, image, false), newImage,
				(int)operation->getArgumentNumeric(ArgumentLabel::X), (int)operation->getArgumentNumeric(ArgumentLabel::Y),
				(int)operation->getArgumentNumeric(ArgumentLabel::Width), (int)operation->getArgumentNumeric(ArgumentLabel::Height));
			newImageSet = true;
			break;

		case ScriptOperationType::Mask:
			ImageOperations::mask(*image, *imageList->getImage(operation->getArgument()), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Grayscale:
			ImageOperations::convertToGrayScale(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Color:
			ImageOperations::convertToColor(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ColorAlpha:
			ImageOperations::convertToColorAlpha(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetSaturation:
			ImageOperations::getSaturation(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsValue:
			ImageOperations::getHsValue(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsLightness:
			ImageOperations::getHsLightness(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Threshold:
			ImageOperations::threshold(*getLabelOrCurrentImage(operation, image, false), *newImage, operation->getArgumentNumeric());
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
			ImageOperations::multiply(*getLabelOrCurrentImage(operation, image, false), operation->getArgumentNumeric(), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Invert:
			ImageOperations::invert(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;
			/*
			case ScriptOperationType::RollingBall:
				newImage = BackgroundSubtract::subtract_background_rolling_ball(getLabelOrCurrentImage(operation, image, false), 50, false);
				newImageSet = true;
				break;
			*/
		case ScriptOperationType::UpdateBackground:
			backgroundBuffer->addImage(getLabelOrCurrentImage(operation, image, true), operation->getArgumentNumeric());
			backgroundBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateAverage:
			averageBuffer->addImage(getLabelOrCurrentImage(operation, image, true), operation->getArgumentNumeric());
			averageBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ClearSeries:
			imageSeries->reset();
			break;

		case ScriptOperationType::AddSeries:
			imageSeries->addImage(getLabelOrCurrentImage(operation, image, true), (int)operation->getArgumentNumeric());
			break;

		case ScriptOperationType::GetSeriesMedian:
			newImageSet = imageSeries->getMedian(*newImage, observer);
			break;

		case ScriptOperationType::AddAccum:
			accumBuffer->addImage(getLabelOrCurrentImage(operation, image, true), (AccumMode)operation->getArgument(ArgumentLabel::AccumMode, AccumMode::Age));
			break;

		case ScriptOperationType::GetAccum:
			logPower = operation->getArgumentNumeric();
			logPalette = (Palette)operation->getArgument(ArgumentLabel::Palette, Palette::Grayscale);
			accumBuffer->getImage(newImage, (float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::CreateClusters:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker), true)->createClusters(image, operation->getArgumentNumeric(ArgumentLabel::MinArea), operation->getArgumentNumeric(ArgumentLabel::MaxArea), basepath, debugMode);
			break;

		case ScriptOperationType::CreateTracks:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->createTracks(operation->getArgumentNumeric(ArgumentLabel::MaxMove), (int)operation->getArgumentNumeric(ArgumentLabel::MinActive), (int)operation->getArgumentNumeric(ArgumentLabel::MaxInactive), basepath);
			break;

		case ScriptOperationType::CreatePaths:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->createPaths(operation->getArgumentNumeric(ArgumentLabel::Distance));
			break;

		case ScriptOperationType::DrawClusters:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->drawClusters(getLabelOrCurrentImage(operation, image, true), newImage, operation->getClusterDrawMode(ClusterDrawMode::ClusterDefault));
			newImageSet = true;
			break;

		case ScriptOperationType::DrawTracks:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->drawTracks(getLabelOrCurrentImage(operation, image, true), newImage, operation->getClusterDrawMode(ClusterDrawMode::TracksDefault), (int)sourceFps);
			newImageSet = true;
			break;

		case ScriptOperationType::DrawPaths:
			logPower = operation->getArgumentNumeric();
			logPalette = (Palette)operation->getArgument(ArgumentLabel::Palette, Palette::Grayscale);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->drawPaths(getLabelOrCurrentImage(operation, image, true), newImage,
				(PathDrawMode)operation->getArgument(ArgumentLabel::PathDrawMode, PathDrawMode::Age), (float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::ShowTrackInfo:
			emit showInfo(imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->getInfo().c_str(), (int)operation->getArgumentNumeric(ArgumentLabel::Display, true));
			break;

		case ScriptOperationType::DrawTrackInfo:
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->drawTrackInfo(getLabelOrCurrentImage(operation, image, true), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::SaveClusters:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->saveClusters(outputPath.createFilePath(frame), frame, getTime(frame), (SaveFormat)operation->getArgument(ArgumentLabel::Format, SaveFormat::ByTime), operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SaveTracks:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->saveTracks(outputPath.createFilePath(frame), frame, getTime(frame), (SaveFormat)operation->getArgument(ArgumentLabel::Format, SaveFormat::ByTime), operation->getArgumentBoolean(ArgumentLabel::Contour));
			break;

		case ScriptOperationType::SavePaths:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->savePaths(outputPath.createFilePath(frame), frame, getTime(frame));
			break;

		case ScriptOperationType::SaveTrackInfo:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->saveTrackInfo(outputPath.createFilePath(frame), frame, getTime(frame));
			break;

		case ScriptOperationType::SaveTrackLog:
			outputPath.setOutputPath(basepath, operation->getArgument(ArgumentLabel::Path), Constants::defaultDataExtension);
			imageTrackers->getTracker(observer, operation->getArgument(ArgumentLabel::Tracker))->initLogClusterTrack(outputPath.createFilePath(count));
			break;

		case ScriptOperationType::DrawLegend:
			displayi = (int)operation->getArgumentNumeric(ArgumentLabel::Display);
			if (displayi > 0) {
				// show in separate display
				ImageOperations::create(newImage, 1000, 1000);
				ImageOperations::drawLegend(*newImage, *newImage, DrawPosition::Full, logPower, logPalette);
				emit showImage(newImage, displayi - 1);		// manual one-base correction
			} else {
				// draw on image
				ImageOperations::drawLegend(*getLabelOrCurrentImage(operation, image, true), *newImage, (DrawPosition)operation->getArgument(ArgumentLabel::Position, DrawPosition::BottomRight), logPower, logPalette);
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
			break;

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
		string errorMsg;
#ifdef _DEBUG
		errorMsg = e.msg;
#else
		if (e.err != "") {
			errorMsg = e.err;
		} else {
			errorMsg = e.msg;
		}
#endif
		if (Util::contains(errorMsg, "==")) {
			// adding more user friendly messages:
			if (Util::contains(errorMsg, "CV_MAT_TYPE")) {
				errorMsg += " (Image types don't match)";
			}
			if (Util::contains(errorMsg, "CV_MAT_CN")) {
				errorMsg += " (Image color channels don't match)";
			}
		}
		errorMsg += " in\n" + operation->line;
		cerr << e.what() << endl;
		emit showDialog(errorMsg.c_str());
		doAbort();
	} catch (std::exception e) {
		string errorMsg = Util::getExceptionDetail(e) + " in\n" + operation->line;
		cerr << e.what() << endl;
		emit showDialog(errorMsg.c_str());
		doAbort();
	}
	return done;
}

Mat* ScriptProcessing::getLabelOrCurrentImage(ScriptOperation* operation, Mat* currentImage, bool explicitArgument) {
	Mat* image;
	string label;

	if (explicitArgument) {
		label = operation->getArgument(ArgumentLabel::Label);
	} else {
		label = operation->getArgument();
	}
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
	abort = true;

	imageTrackers->close();
	scriptOperations->close();

	observer->resetProgressTimer();
	emit resetUI();
	emit clearStatus();
}
