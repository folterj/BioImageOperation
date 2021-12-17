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

void ImageSeries::reset() {
	images.clear();
	nchannels = 0;
	type = 0;
	width = 0;
	height = 0;
}

void ImageSeries::addImage(Mat* image, int bufferSize) {
	vector<Mat> channels;

	if (images.empty()) {
		nchannels = image->channels();
		type = image->type();
		width = image->cols;
		height = image->rows;
	} else if (image->channels() != nchannels) {
		throw invalid_argument("Number of image channels does not match current image series");
	} else if (image->cols != width || image->rows != height) {
		throw invalid_argument("Image size does not match current image series");
	}

	if (bufferSize != 0) {
		// remove oldest image(s)
		while (images.size() >= bufferSize) {
			images.pop_front();
		}
	}

	vector<vector<uchar>> vimage(nchannels);

	split(*image, channels);
	for (int c = 0; c < nchannels; c++) {
		channels[c].reshape(0, 1).copyTo(vimage[c]);
	}
	images.push_back(vimage);
}

bool ImageSeries::getMedian(OutputArray dest, MedianMode mode) {
	Mat image;
	uchar* outData;
	int pixeli;
	int npixels = width * height;
	int n = (int)images.size();
	int m;
	int maxm = int(round(n * 0.99));
	int minm = int(round(n * 0.01));
	uchar median, low, high, dlow, dhigh;
	vector<uchar> pixelBuffer(n);

	if (n == 0) {
		return false;
	}

	dest.create(height, width, type);
	image = dest.getMat();
	outData = (uchar*)image.data;

	for (pixeli = 0; pixeli < npixels; pixeli++) {
		for (int c = 0; c < nchannels; c++) {
			for (int i = 0; i < n; i++) {
				pixelBuffer[i] = images[i][c][pixeli];
			}
			m = n / 2;
			sort(pixelBuffer.begin(), pixelBuffer.end());
			median = pixelBuffer[m];
			if (n % 2 == 0) {
				// even number of images: average of 2 middle elements
				m--;
				median = (median + pixelBuffer[m]) / 2;
			}
			if (mode != MedianMode::Normal) {
				// move towards lighter / darker side of array
				low = pixelBuffer.front();
				high = pixelBuffer.back();
				do {
					median = pixelBuffer[m];
					dlow = median - low;
					dhigh = high - median;
					if (mode == MedianMode::Light) {
						m++;
					} else if (mode == MedianMode::Dark) {
						m--;
					}
				} while (((mode == MedianMode::Light && dlow < 10 * dhigh) || (mode == MedianMode::Dark && dhigh < 10 * dlow)) && m >= minm && m < maxm);

			}
			outData[pixeli * nchannels + c] = median;
		}
	}
	return true;
}

bool ImageSeries::getMean(OutputArray dest) {
	Mat image;
	uchar* outData;
	int pixeli;
	int npixels = width * height;
	int n = (int)images.size();
	double sum, mean;

	if (n == 0) {
		return false;
	}

	dest.create(height, width, type);
	image = dest.getMat();
	outData = (uchar*)image.data;

	for (pixeli = 0; pixeli < npixels; pixeli++) {
		for (int c = 0; c < nchannels; c++) {
			sum = 0;
			for (int i = 0; i < n; i++) {
				sum += images[i][c][pixeli];
			}
			mean = sum / n;
			outData[pixeli * nchannels + c] = (uchar)mean;
		}
	}
	return true;
}
