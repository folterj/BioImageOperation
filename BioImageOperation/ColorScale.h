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

/*
 * Basic BGR structure for efficient memory mapping
 */

struct BGR
{
	unsigned char b, g, r;
};


/*
 * Pre-calculated palettes for fast color mapping
 */

class ColorScale
{
public:
	static const int colorLevels = 16;											// 256 -> significant memory; anything from 8 actually looks fine
	static const int scaleLength = colorLevels * colorLevels * colorLevels;

	static BGR grayTable[];
	static BGR heatTable[];
	static BGR rainbowTable[];

	static void init();
	static BGR getGrayScale(float scale);
	static BGR getHeatScale(float scale);
	static BGR getRainbowScale(float scale);
};
