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

#pragma once
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

#include "Constants.h"

using namespace System;
using namespace cv;


/*
 * Image operations
 */

class ImageOperations
{
public:
	static void create(Mat* image, int width, int height, ImageColorMode colorMode = ImageColorMode::Color, double r = 0, double g = 0, double b = 0);
	static void convertToGrayScale(InputArray source, OutputArray dest);
	static void convertToColor(InputArray source, OutputArray dest);
	static void convertToColorAlpha(InputArray source, OutputArray dest);
	static void getSaturation(InputArray source, Mat* dest);
	static void getHsValue(InputArray source, Mat* dest);
	static void getHsLightness(InputArray source, Mat* dest);

	static void scale(InputArray source, OutputArray dest, int width, int height);
	static void crop(Mat* source, Mat* dest, int x, int y, int width, int height);
	static void mask(InputArray source, InputArray mask, OutputArray dest);
	static void threshold(InputArray source, OutputArray dest, double thresh = 0);
	static void difference(InputArray source1, InputArray source2, OutputArray dest, bool abs = false);
	static void add(InputArray source1, InputArray source2, OutputArray dest);
	static void multiply(InputArray source, double factor, OutputArray dest);
	static void invert(InputArray source, OutputArray dest);

	static void drawLegend(InputArray source, OutputArray dest, DrawPosition position, double logPower, Palette palette);
	static void drawColorScale(Mat* dest, Rect rect, double logPower, Palette palette);
	static void drawColorSwatches(Mat* dest, Rect rect);
};
