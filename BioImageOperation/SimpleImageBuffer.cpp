/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "SimpleImageBuffer.h"


SimpleImageBuffer::SimpleImageBuffer() {
}

void SimpleImageBuffer::reset() {
	set = false;
}

void SimpleImageBuffer::setImage(Mat* image) {
	image->convertTo(bufferImage, CV_32F);
	set = true;
}

void SimpleImageBuffer::addWeighted(Mat* image, Mat* result, double weight) {
	if (!set) {
		// initialise using current image (uning setImage seems to change step size for some reason)
		image->convertTo(bufferImage, CV_32F);
		set = true;
	} else {
		// add using weighting
		if (weight == 0) {
			weight = 0.01;
		}
		accumulateWeighted(*image, bufferImage, weight);
	}
	bufferImage.convertTo(*result, CV_8U);
}

void SimpleImageBuffer::addMin(Mat* image, Mat* result) {
	if (!set) {
		// initialise using current image (uning setImage seems to change step size for some reason)
		image->copyTo(bufferImage);
		set = true;
	} else {
		bufferImage = min(*image, bufferImage);
	}
	bufferImage.copyTo(*result);
}

void SimpleImageBuffer::addMax(Mat* image, Mat* result) {
	if (!set) {
		// initialise using current image (uning setImage seems to change step size for some reason)
		image->copyTo(bufferImage);
		set = true;
	} else {
		bufferImage = max(*image, bufferImage);
	}
	bufferImage.copyTo(*result);
}
