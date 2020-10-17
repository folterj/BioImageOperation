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
#include "FrameOutput.h"
#include "NumericPath.h"

using namespace cv;


/*
 * Output to video file
 */

class VideoOutput : public FrameOutput
{
public:
	VideoWriter videoWriter;
	bool videoIsOpen = false;
	NumericPath outputPath;
	int width = 0;
	int height = 0;
	bool isColor = true;
	double fps = 0;
	int codec = 0;

	VideoOutput();
	~VideoOutput();
	void init(string basePath, string filePath, string defaultExtension = "", string start = "", string length = "", double fps = 0, string codecs = "");
	void reset();
	bool open();
	void close();
	bool writeImage(Mat* image);
};
