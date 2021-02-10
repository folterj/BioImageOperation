/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


/*
 * Base class for image source classes
 */

class FrameSource
{
protected:
	int start = 0;
	int end = 0;
	int interval = 1;
	double fps = 1;

public:
	virtual void reset() = 0;
	virtual bool init(int apiCode, string basepath, string filepath, string start = "", string length = "", double fps = 1, int interval = 1, int total = 0) = 0;
	virtual bool getNextImage(Mat* image) = 0;
	virtual void close() = 0;

	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual double getFps() = 0;
	virtual int getFrameNumber() = 0;

	virtual string getLabel() = 0;
	virtual int getCurrentFrame() = 0;
	virtual int getTotalFrames() = 0;

	void calcFrameParams(string start, string length, double fps, int interval, int total, int nframes);
};
