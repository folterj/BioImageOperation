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
 * Pre-calculated palettes for fast color mapping
 */

class ColorScale
{
public:
	static const int colorLevels = 16;											// 256 -> significant memory; anything from 8 actually looks fine
	static const int scaleLength = colorLevels * colorLevels * colorLevels;
	static const int labelLength = 1000;

	static Vec<unsigned char, 3> grayTable[];
	static Vec<unsigned char, 3> heatTable[];
	static Vec<unsigned char, 3> rainbowTable[];
	static Vec<unsigned char, 3> labelTable[];

	static void init();
	static Vec<unsigned char, 3> getGrayScale(double scale);
	static Vec<unsigned char, 3> getHeatScale(double scale);
	static Vec<unsigned char, 3> getRainbowScale(double scale);
	static Vec<unsigned char, 3> getLabelColor(int label);
};
