/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageSeries.h"
#include "ImageOperations.h"


ImageSeries::ImageSeries() {
}

ImageSeries::~ImageSeries() {
}

void ImageSeries::reset() {
	images.clear();
	width = 0;
	height = 0;
}

void ImageSeries::addImage(Mat* image, int bufferSize) {
	if (width == 0 || height == 0) {
		width = image->cols;
		height = image->rows;
	} else if (image->cols != width || image->rows != height) {
		throw invalid_argument("Image size does not match current image series");
	}

	if (bufferSize != 0) {
		// remove oldest image(s)
		while (images.size() >= bufferSize) {
			images.pop_front();
		}
	}
	images.push_back(image->clone());
}

bool ImageSeries::getMedian(OutputArray dest, Observer* observer) {
	Mat image;
	unsigned char* outData;
	unsigned char* inData;
	int pixeli;
	int m;
	unsigned char median;
	int n = (int)images.size();
	vector<unsigned char> pixelBuffer(n);

	if (n == 0) {
		return false;
	}

	// Currently only works for GrayScale
	for (int i = 0; i < n; i++) {
		if (images[i].channels() > 1) {
			ImageOperations::convertToGrayScale(images[i], images[i]);
		}
	}

	dest.create(height, width, CV_8U);
	image = dest.getMat();
	outData = (unsigned char*)image.data;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			pixeli = y * width + x;
			for (int i = 0; i < n; i++) {
				inData = images[i].data;
				pixelBuffer[i] = inData[pixeli]; // * slow
			}
			sort(pixelBuffer.begin(), pixelBuffer.end()); // * very slow
			m = n / 2;
			if (n % 2 != 0) {
				// odd number: single middle element
				median = pixelBuffer[m];
			} else {
				// even number: two middle elements
				median = (pixelBuffer[m] + pixelBuffer[m - 1]) / 2;
			}
			outData[y * width + x] = median;
		}
		if (observer) {
			observer->showStatus(y, height);
		}
	}
	return true;
}
