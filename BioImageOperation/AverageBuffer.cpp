/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "AverageBuffer.h"


AverageBuffer::AverageBuffer() {
}

void AverageBuffer::reset() {
	set = false;
}

void AverageBuffer::addImage(Mat* image, double weight) {
	if (!set) {
		// initialise background using current image
		image->convertTo(averageImage, CV_32F);
		set = true;
	} else {
		// add using weighting
		if (weight == 0) {
			weight = 0.01;
		}
		accumulateWeighted(*image, averageImage, weight);
	}
}

void AverageBuffer::getImage(Mat* dest) {
	averageImage.convertTo(*dest, CV_8U);
}
