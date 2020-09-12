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
	virtual void showStatus(int i) = 0;
	virtual void showStatus(int i, int tot) = 0;
	virtual void showStatus(string label, int i, int tot, bool showFrameProgress) = 0;
	virtual void showStatus(string status, double progress) = 0;
	virtual void showInfo(string info, int displayi) = 0;
	virtual void displayImage(Mat* image, int displayi) = 0;
	virtual void showErrorMessage(string message) = 0;
};
