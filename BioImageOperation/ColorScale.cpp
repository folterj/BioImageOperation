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

#include "ColorScale.h"
#include "Util.h"


BGR ColorScale::grayTable[scaleLength];
BGR ColorScale::heatTable[scaleLength];
BGR ColorScale::rainbowTable[scaleLength];

void ColorScale::init()
{
	// initialise lookup table
	double scale;
	Scalar color;

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
