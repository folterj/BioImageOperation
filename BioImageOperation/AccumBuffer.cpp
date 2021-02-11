/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "AccumBuffer.h"
#include "ColorScale.h"


AccumBuffer::AccumBuffer() {
}

void AccumBuffer::reset() {
	set = false;
	total = 0;
}

void AccumBuffer::create(int width, int height) {
	bufferImage.create(height, width, CV_32F);
	bufferImage.setTo(0);

	helpImage.create(height, width, CV_32F);
	helpImage.setTo(0);

	total = 0;
	set = true;
}

void AccumBuffer::addImage(Mat* image, AccumMode accumMode) {
	// assume binary image
	this->accumMode = accumMode;

	if (!set) {
		create(image->cols, image->rows);
	}

	if (accumMode == AccumMode::Usage) {
		helpImage.setTo(1);							// binary image has values 0 or 255 -> instead use 0 or 1
		accumulate(helpImage, bufferImage, *image);	// use image as mask (add)
	} else if (accumMode == AccumMode::Age) {
		helpImage.setTo(total);
		helpImage.copyTo(bufferImage, *image);		// use image as mask (replace)
	}
	total++;
}

void AccumBuffer::getImage(Mat* dest, float power, Palette palette) {
	Mat image;
	float* inData;
	unsigned char* outData;
	int width = bufferImage.cols;
	int height = bufferImage.rows;
	float val, scale, colScale;
	BGR color;
	int pixeli;

	if (power == 0) {
		power = 1;
	}

	inData = (float*)bufferImage.data;

	dest->create(height, width, CV_8UC3);
	outData = (unsigned char*)dest->data;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			pixeli = y * width + x;

			val = inData[pixeli];
			// process any non-zero pixel
			if (val != 0) {
				switch (accumMode) {
				case AccumMode::Age: scale = (float)1 / (total - val); break;
				case AccumMode::Usage: scale = (float)val / total; break;
				}
				// 	colScale: 0...1
				colScale = -log10(scale) / power;		// log: 1(E0) ... 1E-[power]

				if (colScale < 0) {
					colScale = 0;
				}
				if (colScale > 1) {
					colScale = 1;
				}

				switch (palette) {
				case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
				case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
				default: color = ColorScale::getGrayScale(colScale); break;
				}
				outData[pixeli * 3 + 0] = color.b;
				outData[pixeli * 3 + 1] = color.g;
				outData[pixeli * 3 + 2] = color.r;
			}
		}
	}
}
