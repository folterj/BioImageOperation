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
#include "Constants.h"

using namespace cv;


/*
 * Image operations
 */

class ImageOperations
{
public:
	static void create(Mat* image, int width, int height, ArgumentValue colorMode = ArgumentValue::Color, double r = 0, double g = 0, double b = 0);
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

	static void drawLegend(InputArray source, OutputArray dest, ArgumentValue position, double logPower, ArgumentValue palette);
	static void drawColorScale(Mat* dest, Rect rect, double logPower, ArgumentValue palette);
	static void drawColorSwatches(Mat* dest, Rect rect);
};
