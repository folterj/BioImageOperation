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
#include "FrameSource.h"
#include "NumericPath.h"

using namespace std;
using namespace cv;


/*
 * Source for video file(s)
 */

class VideoSource : public FrameSource
{
public:
	VideoCapture videoCapture;
	bool videoIsOpen = false;
	int apiCode = 0;
	NumericPath sourcePath;
	string label = "";
	int nsources = 0;
	int sourcei = 0;
	int nframes = 0;
	int videoNframes = 0;
	int framei = 0;
	int videoFramei = 0;
	int width = 0;
	int height = 0;
	bool seekMode = false;

	VideoSource();
	~VideoSource();
	void reset();
	bool init(string basepath, string filepath, int apiCode, string codecs = "", string start = "", string length = "",
			  double fps = 1, int interval = 1, int total = 0, int width=0, int height=0);
	bool open();
	void release();
	void close();
	bool getNextImage(Mat* image);
	bool seekFrame();
	bool nextFrame();

	int getWidth();
	int getHeight();
	double getFps();
	int getFrameNumber();

	string getLabel();
	int getCurrentFrame();
	int getTotalFrames();
};
