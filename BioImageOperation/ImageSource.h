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
#include "FrameSource.h"
#include "NumericPath.h"

using namespace std;
using namespace cv;


/*
 * Source for image file(s)
 */

class ImageSource : public FrameSource
{
public:
	NumericPath sourcePath;
	string label = "";
	int width = 0;
	int height = 0;
	int nfiles = 0;
	int filei = 0;

	ImageSource();
	~ImageSource();
	void reset();
	bool init(string basepath, string filepath, int apiCode, string codecs = "", string start = "", string length = "",
			  double fps = 1, int interval = 1, int total = 0, int width = 0, int height = 0);
	bool open();
	bool getNextImage(Mat* image);
	void close();

	int getWidth();
	int getHeight();
	double getFps();
	int getFrameNumber();

	string getLabel();
	int getCurrentFrame();
	int getTotalFrames();
};
