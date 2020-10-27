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
//#include "ImageOperations.h"
#include "NumericPath.h"
#include "Constants.h"
#include "Util.h"


ScriptProcessing::ScriptProcessing() {
	// initialise static lookup tables
	ColorScale::init();

	//ScriptOperation::writeOperationList("D:\\BioImageOperation.rtf");
}

ScriptProcessing::~ScriptProcessing() {

}

void ScriptProcessing::reset() {
	basepath = "";
	sourceWidth = 0;
	sourceHeight = 0;
	sourceFps = 0;
	sourceFrameNumber = 0;
	logPower = 0;
	running = false;
	abort = false;

	scriptOperations->reset();
	//imageList->reset();
	//backgroundBuffer->reset();
	//averageBuffer->reset();
	//imageSeries->reset();
	//accumBuffer->reset();

	//imageTrackers->reset();
	emit resetImages();
}

void ScriptProcessing::registerObserver(Observer* observer) {
	this->observer = observer;
}

bool ScriptProcessing::startProcess(string filepath, string script) {
	reset();
	basepath = Util::extractFilePath(filepath);

	try {
		scriptOperations->extract(script, 0);
		observer->resetProgressTimer();

		if (processThread.joinable()) {
			processThread.join();
		}

		connect(this, &ScriptProcessing::resetUI, (MainWindow*)observer, &MainWindow::resetUI);
		connect(this, &ScriptProcessing::resetImages, (MainWindow*)observer, &MainWindow::resetImages);
		connect(this, &ScriptProcessing::clearStatus, (MainWindow*)observer, &MainWindow::clearStatus);
		connect(this, &ScriptProcessing::showStatus, (MainWindow*)observer, &MainWindow::showStatus);
		connect(this, &ScriptProcessing::showInfo, (MainWindow*)observer, &MainWindow::showInfo);
		connect(this, &ScriptProcessing::showError, (MainWindow*)observer, &MainWindow::showError);
		connect(this, &ScriptProcessing::showImage, (MainWindow*)observer, &MainWindow::showImage);

		//processThread = QThread::create(&ScriptProcessing::processThreadMethod, this);
		//processThread->start();
		processThread = std::thread(&ScriptProcessing::processThreadMethod, this);
	} catch (exception e) {
		emit showError(e.what());
		doAbort(false);
		return false;
	}
	return true;
}

void ScriptProcessing::processThreadMethod() {
	running = true;
	processOperations(scriptOperations, NULL);
	doAbort(false);
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

		case  ScriptOperationType::Debug:
			debugMode = true;
			break;

		case  ScriptOperationType::OpenVideo:
			operation->initFrameSource(FrameType::Video, (int)operation->getArgumentNumeric(ArgumentLabel::API), basepath, operation->getArgument(ArgumentLabel::Path), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), 0, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				if (observer->checkStatusProcess()) {
					emit showStatus(operation->frameSource->getLabel().c_str(), operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames());
				}
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

		case ScriptOperationType::ShowImage:
			//refImage = getLabelOrCurrentImage(operation, image, false);
			refImage = image;

			if (Util::isValidImage(refImage)) {
				//observer->displayImage(refImage, (int)operation->getArgumentNumeric(ArgumentLabel::None, true));
				if (observer->checkImageProcess()) {
					emit showImage(refImage, (int)operation->getArgumentNumeric(ArgumentLabel::None, true));
				}
			}
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
				//imageList->setImage(newImage, operation->asignee);
			} else {
				//imageList->setImage(image, operation->asignee);
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
		emit showError(errorMsg.c_str());
		doAbort(false);
	} catch (exception e) {
		string errorMsg = string(e.what()) + " in\n" + operation->line;
#ifdef _DEBUG
		//errorMsg += e->StackTrace;
#endif
		cerr << e.what() << endl;
		emit showError(errorMsg.c_str());
		doAbort(false);
	}
	return done;
}

void ScriptProcessing::doAbort(bool tryKill) {
	abort = true;

	if (tryKill) {
		//if (running) {
		//	processThread.~thread();
		//}
	}

	//imageTrackers->close();
	scriptOperations->close();

	emit resetUI();
	emit clearStatus();

	running = false;
}
