/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ColorScale.h"
#include "Util.h"


BGR ColorScale::grayTable[scaleLength];
BGR ColorScale::heatTable[scaleLength];
BGR ColorScale::rainbowTable[scaleLength];

void ColorScale::init()
{
	// initialise lookup table
	double scale;
	cv::Scalar color;

	for (int i = 0; i < scaleLength; i++)
	{
		scale = (double)i / scaleLength;

		grayTable[i].b = (unsigned char)((1 - scale) * 0xFF);
		grayTable[i].g = (unsigned char)((1 - scale) * 0xFF);
		grayTable[i].r = (unsigned char)((1 - scale) * 0xFF);

		color = Util::getHeatScale(scale);
		heatTable[i].b = (unsigned char)color[2];
		heatTable[i].g = (unsigned char)color[1];
		heatTable[i].r = (unsigned char)color[0];

		color = Util::getRainbowScale(scale);
		rainbowTable[i].b = (unsigned char)color[2];
		rainbowTable[i].g = (unsigned char)color[1];
		rainbowTable[i].r = (unsigned char)color[0];
	}
}

BGR ColorScale::getGrayScale(float scale)
{
	int i;

	if (scale < 0)
	{
		i = 0;
	}
	else if (scale >= 1)
	{
		i = scaleLength - 1;
	}
	else
	{
		i = (int)(scale * scaleLength);
	}

	return grayTable[i];
}

BGR ColorScale::getHeatScale(float scale)
{
	int i;

	if (scale < 0)
	{
		i = 0;
	}
	else if (scale >= 1)
	{
		i = scaleLength - 1;
	}
	else
	{
		i = (int)(scale * scaleLength);
	}

	return heatTable[i];
}

BGR ColorScale::getRainbowScale(float scale)
{
	int i;

	if (scale < 0)
	{
		i = 0;
	}
	else if (scale >= 1)
	{
		i = scaleLength - 1;
	}
	else
	{
		i = (int)(scale * scaleLength);
	}

	return rainbowTable[i];
}
