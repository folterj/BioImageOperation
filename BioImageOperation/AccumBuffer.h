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

using namespace cv;


/*
 * Class for calculating accumulative image
 */

class AccumBuffer
{
public:
	Mat bufferImage, helpImage;
	AccumMode accumMode = AccumMode::Age;
	int total;
	bool set = false;

	AccumBuffer();
	void reset();
	void create(int width, int height);
	void addImage(Mat* image, AccumMode accumMode);
	void getImage(Mat* dest, float power, Palette palette);
};
