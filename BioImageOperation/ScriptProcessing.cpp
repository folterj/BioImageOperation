/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ScriptProcessing.h"
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
	basePath = "";
	sourceWidth = 0;
	sourceHeight = 0;
	sourceFps = 0;
	sourceFrameNumber = 0;
	logPower = 0;
	abort = false;

	scriptOperations->reset();
	//imageList->reset();
	//backgroundBuffer->reset();
	//averageBuffer->reset();
	//imageSeries->reset();
	//accumBuffer->reset();

	//imageTrackers->reset();
	observer->resetImages();
}

void ScriptProcessing::registerObserver(Observer* observer) {
	this->observer = observer;
}

bool ScriptProcessing::startProcess(string filePath, string script) {
	reset();
	basePath = Util::extractFilePath(filePath);

	try {
		scriptOperations->extract(script, 0);
		observer->resetProgressTimer();

		processThread = thread(&ScriptProcessing::processThreadMethod, this);
	} catch (exception e) {
		observer->showErrorMessage(e.what());
		doAbort(true);
		return false;
	}
	return true;
}

void ScriptProcessing::processThreadMethod() {
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
			basePath = operation->getArgument("Path");
			break;
		case  ScriptOperationType::Debug:
			debugMode = true;
			break;
		case  ScriptOperationType::OpenVideo:
			operation->initFrameSource(FrameType::Video, (int)operation->getArgumentNumeric("API"), basePath, operation->getArgument("Path"), operation->getArgument("Start"), operation->getArgument("Length"), 0, (int)operation->getArgumentNumeric("Interval"));
			sourceFrameNumber = operation->frameSource->getFrameNumber();
			if (operation->frameSource->getNextImage(newImage)) {
				observer->showStatus(operation->frameSource->getLabel(), operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), true);
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
		observer->showErrorMessage(errorMsg);
		doAbort(false);
	} catch (exception e) {
		string errorMsg = string(e.what()) + " in\n" + operation->line;
#ifdef _DEBUG
		//errorMsg += e->StackTrace;
#endif
		observer->showErrorMessage(errorMsg);
		doAbort(false);
	}
	return done;
}

void ScriptProcessing::doAbort(bool tryKill) {
	abort = true;

	if (tryKill) {
		if (processThread.joinable()) {
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	//imageTrackers->close();
	scriptOperations->close();

	observer->resetUI();
	observer->clearStatus();

	exit(0);	// force threads to terminate as well
}
