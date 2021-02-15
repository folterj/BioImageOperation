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

class SimpleImageBuffer
{
public:
	Mat bufferImage;
	bool set = false;

	SimpleImageBuffer();
	void reset();
	void setImage(Mat* image);
	void addWeighted(Mat* image, Mat* result, double weight);
	void addMin(Mat* image, Mat* result);
	void addMax(Mat* image, Mat* result);
};
