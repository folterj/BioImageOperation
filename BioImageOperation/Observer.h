/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <opencv2/opencv.hpp>
#include "Constants.h"
#include "ScriptOperations.h"

using namespace std;
using namespace cv;


/*
 * Observer interface to update back to the UI
 */

class Observer
{
public:
	virtual void requestPause() = 0;
	virtual void setMode(int mode) = 0;
	virtual void resetProgressTimer() = 0;
	virtual void clearStatus() = 0;
	virtual bool checkStatusProcess() = 0;
	virtual void showStatus(int i, int tot = 0, string label = "") = 0;
	virtual bool checkOperationsProcess() = 0;
	virtual void showOperations(ScriptOperations* operations, ScriptOperation* currentOperation) = 0;
	virtual void showDialog(string message, int level = (int)MessageLevel::Info) = 0;
	virtual bool checkTextProcess(int displayi) = 0;
	virtual void showText(string text, int displayi, string reference = "") = 0;
	virtual bool checkImageProcess(int displayi) = 0;
	virtual void showImage(Mat* image, int displayi, string reference = "") = 0;
};
