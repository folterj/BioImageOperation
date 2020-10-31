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

using namespace cv;


/*
 * Class to determine average image
 */

class AverageBuffer
{
public:
	Mat averageImage;
	bool set = false;

	AverageBuffer();
	void reset();
	void addImage(Mat* image, double weight);
	void getImage(Mat* image);
};
