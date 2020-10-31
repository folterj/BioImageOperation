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

using namespace std;
using namespace cv;


/*
 * Observer interface to update back to the UI
 */

class Observer abstract
{
public:
	virtual void resetUI() = 0;
	virtual void resetImages() = 0;
	virtual void resetProgressTimer() = 0;
	virtual void clearStatus() = 0;
	virtual bool checkStatusProcess() = 0;
	virtual void showStatus(int i, int tot = 0, const char* label = "") = 0;
	virtual void showInfo(const char* info, int displayi) = 0;
	virtual bool checkImageProcess() = 0;
	virtual void showImage(Mat* image, int displayi) = 0;
	virtual void showDialog(const char* message) = 0;
};
