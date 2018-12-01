/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "Observer.h"

using namespace System::Collections::Generic;
using namespace cv;


/*
 * Storage of (limmited) list of images
 */

class ImageSeries
{
public:
	std::deque<Mat> images;
	int width, height;

	ImageSeries();
	~ImageSeries();

	void reset();
	void addImage(Mat* image, int bufferSize = 0);
	void getMedian(OutputArray dest, Observer^ observer = nullptr);
};
