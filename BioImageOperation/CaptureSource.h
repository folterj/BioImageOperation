/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include "FrameSource.h"

using namespace std;
using namespace cv;


/*
 * Source to access video capture
 */

class CaptureSource : public FrameSource
{
public:
	VideoCapture videoCapture;
	vector<int> capParams;
	string source;
	int apiCode = 0;
	bool videoIsOpen = false;
	int framei = 0;
	int width = 0;
	int height = 0;
	double fps = 0;
	int interval = 1;

	CaptureSource();
	~CaptureSource();
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
