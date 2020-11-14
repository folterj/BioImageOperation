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
 * Base class for image output classes
 */

class FrameOutput abstract
{
public:
	virtual void reset() = 0;
	virtual void init(string basepath, string templatePath, string defaultExtension = "", string start = "", string length = "", double fps = 0, string codecs = "") = 0;
	virtual bool writeImage(Mat* image) = 0;
	virtual void close() = 0;
};
