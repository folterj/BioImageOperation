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
#include "Observer.h"

using namespace std;
using namespace cv;


/*
 * Storage of (limmited) list of images
 */

class ImageSeries
{
public:
	deque<vector<vector<uchar>>> images;
	int nchannels = 0;
	int type = 0;
	int width = 0;
	int height = 0;

	ImageSeries();
	void reset();
	void addImage(Mat* image, int bufferSize = 0);
	bool getMedian(OutputArray dest);
	bool getMean(OutputArray dest);
};
