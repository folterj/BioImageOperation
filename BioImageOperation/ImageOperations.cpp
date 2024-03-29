/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageOperations.h"
#include "Util.h"
#include "ColorScale.h"


void ImageOperations::create(Mat* image, int width, int height, ImageColorMode colorMode, double r, double g, double b) {
	int nchannels = 1;

	if (width == 0 || height == 0) {
		throw invalid_argument("Invalid image dimensions");
	}

	switch (colorMode) {
	case ImageColorMode::Color: nchannels = 3; break;
	case ImageColorMode::ColorAlpha: nchannels = 4; break;
	}

	image->create(height, width, CV_MAKETYPE(CV_8U, nchannels));
	image->setTo(Scalar(b * 0xFF, g * 0xFF, r * 0xFF));
}

void ImageOperations::scale(InputArray source, OutputArray dest, double width, double height) {
	int swidth = source.cols();
	int sheight = source.rows();

	if (width <= 1 && height <= 1) {
		width *= swidth;
		height *= sheight;
	}
	if (width == 0 && height == 0) {
		width = swidth;
		height = sheight;
	} else if (width == 0 && height != 0) {
		width = swidth * height / sheight;
	} else if (height == 0 && width != 0) {
		height = sheight * width / swidth;
	}
	resize(source, dest, cv::Size((int)width, (int)height));
}

void ImageOperations::crop(const Mat& source, Mat* dest, double width, double height, double x, double y) {
	int swidth = source.cols;
	int sheight = source.rows;

	if (x < 1 && y < 1 && width <= 1 && height <= 1) {
		if (x + width > 1) {
			width = 1 - x;
		}
		if (y + height > 1) {
			height = 1 - y;
		}
		width *= swidth;
		height *= sheight;
		x *= swidth;
		y *= sheight;
	}
	if (width == 0) {
		width = swidth - x;
	}
	if (height == 0) {
		height = sheight - y;
	}
	*dest = source(Rect((int)x, (int)y, (int)width, (int)height));
}

void ImageOperations::mask(InputArray source, InputArray mask, OutputArray dest) {
	source.copyTo(dest, mask);
}

void ImageOperations::convertToGrayScale(InputArray source, OutputArray dest) {
	int channels = source.channels();
	if (channels > 1) {
		if (channels == 4) {
			cvtColor(source, dest, ColorConversionCodes::COLOR_BGRA2GRAY);
		} else if (channels == 3) {
			cvtColor(source, dest, ColorConversionCodes::COLOR_BGR2GRAY);
		}
	} else {
		source.copyTo(dest);
	}
}

void ImageOperations::convertToColor(InputArray source, OutputArray dest) {
	int channels = source.channels();
	if (channels == 1) {
		cvtColor(source, dest, ColorConversionCodes::COLOR_GRAY2BGR);
	} else if (channels == 4) {
		cvtColor(source, dest, ColorConversionCodes::COLOR_BGRA2BGR);
	} else {
		source.copyTo(dest);
	}
}


void ImageOperations::convertToColorAlpha(InputArray source, OutputArray dest) {
	int channels = source.channels();
	if (channels == 1) {
		cvtColor(source, dest, ColorConversionCodes::COLOR_GRAY2BGRA);
	} else if (channels == 3) {
		cvtColor(source, dest, ColorConversionCodes::COLOR_BGR2BGRA);
	} else {
		source.copyTo(dest);
	}
}

void ImageOperations::convertToInt(const Mat& source, OutputArray dest) {
	double alpha = 1;
	int depth = source.depth();
	bool isFloat = (depth == CV_16F || depth == CV_32F || depth == CV_64F);
	if (isFloat) {
		alpha = 255.0;
	}
	source.convertTo(dest, CV_8U, alpha);
}

void ImageOperations::convertToFloat(const Mat& source, OutputArray dest) {
	double alpha = 1;
	int depth = source.depth();
	bool isFloat = (depth == CV_16F || depth == CV_32F || depth == CV_64F);
	if (!isFloat) {
		alpha = 1.0 / 255;
	}
	source.convertTo(dest, CV_32F, alpha);
}

void ImageOperations::getHue(InputArray source, OutputArray dest) {
	vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);
	split(hsv, channels);
	channels[0].copyTo(dest);	// hue channel
}

void ImageOperations::getSaturation(InputArray source, OutputArray dest) {
	vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);
	split(hsv, channels);
	channels[1].copyTo(dest);	// saturation channel
}

void ImageOperations::getHsValue(InputArray source, OutputArray dest) {
	vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);
	split(hsv, channels);
	channels[2].copyTo(dest);	// value channel
}

void ImageOperations::getHsLightness(InputArray source, OutputArray dest) {
	vector<Mat> channels;
	Mat hsv;

	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HLS);
	split(hsv, channels);
	channels[1].copyTo(dest);	// lightness channel
}

double ImageOperations::threshold(InputArray source, OutputArray dest, double thresh) {
	ThresholdTypes thresholdType;
	int depth = source.depth();
	double maxval;
	bool isFloat = (depth == CV_16F || depth == CV_32F || depth == CV_64F);

	if (thresh > 0) {
		thresholdType = ThresholdTypes::THRESH_BINARY;
	} else {
		thresholdType = ThresholdTypes::THRESH_OTSU;
	}
	if (isFloat) {
		maxval = 1;
	} else {
		maxval = 0xFF;
	}
	thresh = cv::threshold(source, dest, thresh * maxval, maxval, thresholdType);
	//cv::adaptiveThreshold(source, dest, 0xFF, AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C, ThresholdTypes::THRESH_BINARY, 199, 3);
	return thresh / maxval;
}

void ImageOperations::inrange_hsv(InputArray source, OutputArray dest, double hmin, double hmax, double smin, double smax, double vmin, double vmax) {
	Mat hsv;
	int depth = source.depth();
	bool isFloat = (depth == CV_16F || depth == CV_32F || depth == CV_64F);
	double maxval;
	
	cvtColor(source, hsv, ColorConversionCodes::COLOR_BGR2HSV);

	if (hmax == 0 && hmax == hmin) {
		hmax = 360;
	}
	if (smax == 0 && smax <= smin) {
		smax = 1;
	}
	if (vmax == 0 && vmax <= vmin) {
		vmax = 1;
	}

	hmin /= 360;
	hmax /= 360;
	if (!isFloat) {
		maxval = 0xFF;
		smin *= maxval;
		smax *= maxval;
		vmin *= maxval;
		vmax *= maxval;

		maxval = 180;
		hmin *= maxval;
		hmax *= maxval;
	}
	cv::inRange(hsv, Scalar(hmin, smin, vmin), Scalar(hmax, smax, vmax), dest);
}

void ImageOperations::erode(InputArray source, OutputArray dest, int radius) {
	Mat mat;
	int size = 1 + radius * 2;

	mat = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(size, size));
	cv::erode(source, dest, mat);
}

void ImageOperations::dilate(InputArray source, OutputArray dest, int radius) {
	Mat mat;
	int size = 1 + radius * 2;

	mat = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(size, size));
	cv::dilate(source, dest, mat);
}

void ImageOperations::difference(InputArray source1, InputArray source2, OutputArray dest, bool abs) {
	if (abs) {
		absdiff(source1, source2, dest);
	} else {
		subtract(source1, source2, dest);
	}
}

void ImageOperations::add(InputArray source1, InputArray source2, OutputArray dest) {
	cv::add(source1, source2, dest);
}

void ImageOperations::multiply(InputArray source, double factor, OutputArray dest) {
	cv::multiply(source, factor, dest);
}

void ImageOperations::invert(InputArray source, OutputArray dest) {
	bitwise_not(source, dest);
}

void ImageOperations::drawLegend(InputArray source, OutputArray dest, DrawPosition position, double logPower, Palette palette) {
    Mat dest_image;
	double vwidth = 0.05;
	double vheight = 0.25;
	int width = source.cols();
	int height = source.rows();
	int lwidth = (int)(vwidth * width);
	int lheight = (int)(vheight * height);
	int top, left;

	switch (position) {
	case DrawPosition::TopLeft: left = 0; top = 0; break;
	case DrawPosition::BottomLeft: left = width - lwidth; top = height - lheight; break;
	case DrawPosition::TopRight: left = 0; top = 0; break;
	case DrawPosition::BottomRight: left = width - lwidth; top = height - lheight; break;
	default: left = 0; top = 0; lwidth = width; lheight = height; break;
	}

	source.copyTo(dest);
	dest_image = dest.getMat();

	if (logPower != 0) {
        drawColorScale(&dest_image, Rect(left, top, lwidth, lheight), logPower, palette);
	} else {
        drawColorSwatches(&dest_image, Rect(left, top, lwidth, lheight));
	}
}

void ImageOperations::drawColorScale(Mat* dest, Rect rect, double logPower, Palette palette) {
	Scalar backColor = Scalar(0, 0, 0);
	Scalar infoColor = Scalar(0x80, 0x80, 0x80);
	Scalar color;
	int width, height, textHeight;
	double fontScale;
	int thickness;
	int y, ystart, yend, yrange, xstart, xbar, xline;
	string label = "10";
	string label1;
	float val;
	Size textSize;

	width = rect.width;
	height = rect.height;
	fontScale = width * 0.004;
	thickness = (int)fontScale;
	if (thickness < 1) {
		thickness = 1;
	}

	textSize = getTextSize(label, HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale, thickness, nullptr);
	textHeight = textSize.height;

	ystart = rect.tl().y + textHeight;
	yend = rect.tl().y + height - textHeight;
	yrange = yend - ystart;
	xstart = rect.tl().x;
	xbar = xstart + (int)(0.25 * width);
	xline = xstart + (int)(0.4 * width);

	rectangle(*dest, rect, backColor, LineTypes::FILLED, LineTypes::LINE_AA);

	for (int y = ystart; y < yend; y++) {
		val = (float)(y - ystart) / yrange;
		switch (palette) {
		case Palette::Heat: color = ColorScale::getHeatScale(val); break;
		case Palette::Rainbow: color = ColorScale::getRainbowScale(val); break;
		default: color = ColorScale::getGrayScale(val); break;
		}
		line(*dest, Point(xstart, y), Point(xbar, y), color, 1, LineTypes::LINE_AA);
	}

	for (int i = 0; i <= logPower; i++) {
		y = (int)(ystart + yrange / logPower * i);
		label1 = Util::format("%d", -i);
		putText(*dest, label, Point(xline, (int)(y + textHeight * 0.5)), HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale, infoColor, thickness, LineTypes::LINE_AA);
		putText(*dest, label1, Point((int)(xline + textSize.width), (int)(y - textHeight * 0.25)), HersheyFonts::FONT_HERSHEY_SIMPLEX, fontScale * 0.75, infoColor, thickness, LineTypes::LINE_AA);
		line(*dest, Point(xstart, y), Point(xline, y), infoColor, 1, LineTypes::LINE_AA);
	}
}

void ImageOperations::drawColorSwatches(Mat* dest, Rect rect) {
	Scalar backColor = Scalar(0, 0, 0);
	Scalar infoColor = Scalar(0x80, 0x80, 0x80);
	int width = rect.width;
	int height = rect.height;
	double ymargin = 0.05;
	int ystart = rect.tl().y + (int)(ymargin * height);
	int yend = rect.tl().y + (int)((1 - ymargin) * height);
	int yrange = yend - ystart;
	int xstart = rect.tl().x;
	int xbar = xstart + (int)(0.25 * width);
	int y1, y2;
	int ntotal = 27;

	rectangle(*dest, rect, backColor, LineTypes::FILLED, LineTypes::LINE_AA);

	for (int i = 0; i < ntotal; i++) {
		y1 = (int)(ystart + i * yrange / ntotal) + 1;
		y2 = (int)(ystart + (i + 1) * yrange / ntotal) - 1;
		rectangle(*dest, Point(xstart, y1), Point(xbar, y2), ColorScale::getLabelColor(i), LineTypes::FILLED, LineTypes::LINE_AA);
	}
}
