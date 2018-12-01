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

#include "ImageOperations.h"
#include "Util.h"
#include "ColorScale.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


void ImageOperations::create(Mat* image, int width, int height, ImageColorMode colorMode, double r, double g, double b)
{
	int nchannels = 1;

	if (width == 0 || height == 0)
	{
		throw gcnew ArgumentException("Invalid image dimensions");
	}

	switch (colorMode)
	{
	case ImageColorMode::Color: nchannels = 3; break;
	case ImageColorMode::ColorAlpha: nchannels = 4; break;
	}

	image->create(height, width, CV_MAKETYPE(CV_8U, nchannels));
	image->setTo(Scalar(b * 0xFF, g * 0xFF, r * 0xFF));
}

void ImageOperations::scale(InputArray source, OutputArray dest, int width, int height)
{
	resize(source, dest, cv::Size(width, height));
}

void ImageOperations::crop(Mat* source, Mat* dest, int x, int y, int width, int height)
{
	*dest = (*source)(Rect(x, y, width, height));
}

void ImageOperations::mask(InputArray source, InputArray mask, OutputArray dest)
{
	source.copyTo(dest, mask);
}

void ImageOperations::convertToGrayScale(InputArray source, OutputArray dest)
{
	if (source.channels() > 1)
	{
		if (source.channels() == 4)
		{
			cvtColor(source, dest, ColorConversionCodes::COLOR_BGRA2GRAY);
		}
		else if (source.channels() == 3)
		{
			cvtColor(source, dest, ColorConversionCodes::COLOR_BGR2GRAY);
		}
	}
	else
	{
		source.copyTo(dest);
	}
}

void ImageOperations::convertToColor(InputArray source, OutputArray dest)
{
	if (source.channels() == 1)
	{
		cvtColor(source, dest, ColorConversionCodes::COLOR_GRAY2BGR);
	}
	else if (source.channels() == 4)
	{
		cvtColor(source, dest, ColorConversionCodes::COLOR_BGRA2BGR);
	}
	else
	{
		source.copyTo(dest);
	}
}

void ImageOperations::convertToColorAlpha(InputArray source, OutputArray dest)
{
	if (source.channels() == 1)
	{
		cvtColor(source, dest, ColorConversionCodes::COLOR_GRAY2BGRA);
	}
	else if (source.channels() == 3)
	{
		cvtColor(source, dest, ColorConversionCodes::COLOR_BGR2BGRA);
	}
	else
	{
		source.copyTo(dest);
	}
}

void ImageOperations::getSaturation(InputArray source, Mat* dest)
{
	std::vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);
	split(hsv, channels);
	*dest = channels[1];	// saturation channel
}

void ImageOperations::getHsValue(InputArray source, Mat* dest)
{
	std::vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);
	split(hsv, channels);
	*dest = channels[2];	// value channel
}

void ImageOperations::getHsLightness(InputArray source, Mat* dest)
{
	std::vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HLS);
	split(hsv, channels);
	*dest = channels[1];	// lightness channel
}

void ImageOperations::threshold(InputArray source, OutputArray dest, double thresh)
{
	ThresholdTypes thresholdType;

	if (thresh != 0)
	{
		thresholdType = ThresholdTypes::THRESH_BINARY;
	}
	else
	{
		thresholdType = ThresholdTypes::THRESH_OTSU;
	}

	cv::threshold(source, dest, (thresh * 0xFF), 0xFF, thresholdType);
}

void ImageOperations::difference(InputArray source1, InputArray source2, OutputArray dest, bool abs)
{
	if (abs)
	{
		absdiff(source1, source2, dest);
	}
	else
	{
		subtract(source1, source2, dest);
	}
}

void ImageOperations::add(InputArray source1, InputArray source2, OutputArray dest)
{
	cv::add(source1, source2, dest);
}

void ImageOperations::multiply(InputArray source, double factor, OutputArray dest)
{
	cv::multiply(source, factor, dest);
}

void ImageOperations::invert(InputArray source, OutputArray dest)
{
	bitwise_not(source, dest);
}

void ImageOperations::drawLegend(InputArray source, OutputArray dest, DrawPosition position, double logPower, Palette palette)
{
	double vwidth = 0.05;
	double vheight = 0.25;
	int width = source.cols();
	int height = source.rows();
	int lwidth = (int)(vwidth * width);
	int lheight = (int)(vheight * height);
	int top, left;
	bool isColor = (source.channels() > 1);

	switch (position)
	{
	case DrawPosition::TopLeft: left = 0; top = 0; break;
	case DrawPosition::BottomLeft: left = width - lwidth; top = height - lheight; break;
	case DrawPosition::TopRight: left = 0; top = 0; break;
	case DrawPosition::BottomRight: left = width - lwidth; top = height - lheight; break;
	default: left = 0; top = 0; lwidth = width; lheight = height; break;
	}

	source.copyTo(dest);

	if (logPower != 0)
	{
		drawColorScale(&dest.getMat(), Rect(left, top, lwidth, lheight), logPower, palette);
	}
	else
	{
		drawColorSwatches(&dest.getMat(), Rect(left, top, lwidth, lheight));
	}
}

void ImageOperations::drawColorScale(Mat* dest, Rect rect, double logPower, Palette palette)
{
	Scalar background = Scalar(0, 0, 0);
	Scalar infoColor = Scalar(0x7F, 0x7F, 0x7F);
	BGR color;
	int width, height;
	double fontScale;
	int thickness;
	int y, ystart, yend, yrange, xstart, xbar, xline;
	System::String^ label0 = "10";
	System::String^ label;
	float val;
	Size textSize;

	width = rect.width;
	height = rect.height;
	fontScale = width * 0.004;
	thickness = (int)fontScale;
	if (thickness < 1)
	{
		thickness = 1;
	}

	textSize = getTextSize(Util::stdString(label0), HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale, thickness, NULL);

	ystart = rect.tl().y + textSize.height;
	yend = rect.tl().y + height - textSize.height;
	yrange = yend - ystart;
	xstart = rect.tl().x;
	xbar = xstart + (int)(0.25 * width);
	xline = xstart + (int)(0.4 * width);

	rectangle(*dest, rect.tl(), rect.br(), background, LineTypes::FILLED, LINE_AA);

	for (int y = ystart; y < yend; y++)
	{
		val = (float)(y - ystart) / yrange;
		switch (palette)
		{
		case Palette::Grayscale: color = ColorScale::getGrayScale(val); break;
		case Palette::Heat: color = ColorScale::getHeatScale(val); break;
		case Palette::Rainbow: color = ColorScale::getRainbowScale(val); break;
		}

		line(*dest, Point(xstart, y), Point(xbar, y), Util::bgrtoScalar(color), 1, LINE_AA);
	}

	for (int i = 0; i <= logPower; i++)
	{
		y = (int)(ystart + i * yrange / logPower);
		label = System::String::Format(label0, -i);
		if (i == 0)
		{
			label += "   (1)";
		}
		putText(*dest, Util::stdString(label), Point(xline, (int)(y + textSize.height / 2)), HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale, infoColor, thickness, LINE_AA);
		label = System::String::Format("{0}", -i);
		putText(*dest, Util::stdString(label), Point((int)(xline + textSize.width), y), HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale, infoColor, thickness, LINE_AA);
		line(*dest, Point(xstart, y), Point(xline, y), infoColor, 1, LINE_AA);
	}
}

void ImageOperations::drawColorSwatches(Mat* dest, Rect rect)
{
	Scalar background = Scalar(0, 0, 0);
	Scalar infoColor = Scalar(0x7F, 0x7F, 0x7F);
	Scalar color;
	int width = rect.width;
	int height = rect.height;
	double ymargin = 0.05;
	int ystart = rect.tl().y + (int)(ymargin * height);
	int yend = rect.tl().y + (int)((1 - ymargin) * height);
	int yrange = yend - ystart;
	int xstart = rect.tl().x;
	int xbar = xstart + (int)(0.25 * width);
	int xline = xstart + (int)(0.5 * width);
	int y1, y2;
	double fontScale = width * 0.004;
	int ntotal = 27;

	rectangle(*dest, rect.tl(), rect.br(), background, LineTypes::FILLED, LINE_AA);

	for (int i = 0; i < ntotal; i++)
	{
		y1 = (int)(ystart + i * yrange / ntotal) + 1;
		y2 = (int)(ystart + (i + 1) * yrange / ntotal) - 1;
		color = Util::getLabelColor(i);
		rectangle(*dest, Point(xstart, y1), Point(xbar, y2), color, LineTypes::FILLED, LINE_AA);
	}
}
