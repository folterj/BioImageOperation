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

#include "ImageSeries.h"


ImageSeries::ImageSeries()
{
}

ImageSeries::~ImageSeries()
{
}

void ImageSeries::reset()
{
	images.clear();
}

void ImageSeries::addImage(Mat* image, int bufferSize)
{
	width = image->cols;
	height = image->rows;

	if (bufferSize != 0)
	{
		// remove oldest image(s)
		while (images.size() >= bufferSize)
		{
			images.pop_front();
		}
	}
	images.push_back(image->clone());
}

bool ImageSeries::getMedian(OutputArray dest, Observer^ observer)
{
	Mat image;
	unsigned char* outData;
	unsigned char* inData;
	int pixeli;
	int m;
	unsigned char median;
	int n = (int)images.size();
	std::vector<unsigned char> pixelBuffer(n);

	if (n == 0) {
		return false;
	}

	dest.create(height, width, CV_8U);
	image = dest.getMat();
	outData = (unsigned char*)image.data;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pixeli = y * width + x;
			for (int i = 0; i < n; i++)
			{
				inData = images[i].data;
				pixelBuffer[i] = inData[pixeli]; // * slow
			}
			std::sort(pixelBuffer.begin(), pixelBuffer.end()); // * very slow
			m = n / 2;
			if (n % 2 != 0)
			{
				// odd number: single middle element
				median = pixelBuffer[m];
			}
			else
			{
				// even number: two middle elements
				median = (pixelBuffer[m] + pixelBuffer[m - 1]) / 2;
			}
			outData[y * width + x] = median;
		}
		if (observer)
		{
			observer->showStatus(y, height);
		}
	}
	return true;
}
