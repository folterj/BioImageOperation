/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <chrono>
#include "Observer.h"
#include <opencv2/opencv.hpp>
#include "Types.h"

using namespace std;
using namespace cv;


class TextObserver : public Observer
{
private:
	Clock::time_point time;

public:
	virtual void pause() override;
	virtual void resetProgressTimer() override;
	virtual bool checkStatusProcess() override;
	virtual bool checkTextProcess(int displayi) override;
	virtual bool checkImageProcess(int displayi) override;
	virtual void setMode(int mode) override;
	virtual void clearStatus() override;
	virtual void showStatus(int i, int tot = 0, string label = "") override;
	virtual void showDialog(string message, int level = (int)MessageLevel::Info) override;
	virtual void showText(string text, int displayi, string reference = "") override;
	virtual void showImage(Mat* image, int displayi, string reference = "") override;
};
