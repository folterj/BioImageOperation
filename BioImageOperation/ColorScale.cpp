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


Vec<uchar, 3> ColorScale::grayTable[scaleLength];
Vec<uchar, 3> ColorScale::heatTable[scaleLength];
Vec<uchar, 3> ColorScale::rainbowTable[scaleLength];
Vec<uchar, 3> ColorScale::labelTable[labelLength];


void ColorScale::init() {
	// initialise lookup table
	double scale;
	uchar gray;
	double hue = 0;
	double lightness = 0.5;
	int i;

	for (i = 0; i < scaleLength; i++) {
		scale = (double)i / scaleLength;

		gray = (uchar)((1 - scale) * 0xFF);
		grayTable[i] = Vec<uchar, 3>(gray, gray, gray);

		heatTable[i] = Util::floatToByteColor(Util::getHeatScale(scale));

		rainbowTable[i] = Util::floatToByteColor(Util::getRainbowScale(scale));
	}

	/*
	i = 0;
	double step = 960;
	while (i < labelLength) {
		while (hue < 360 && i < labelLength) {
			labelTable[i] = Util::floatToByteColor(Util::hsvToColor(hue / 360, 1, 1));
			hue += step;
			i++;
		}
		step /= 2;
		hue = step / 2;
	}
	*/

	for (i = 0; i < labelLength; i++) {
		labelTable[i] = Util::floatToByteColor(Util::normColorLightness(Util::hsvToColor(hue, 1, 1), lightness));
		//hue = fmod(hue + 0.618033989, 1);
		hue = fmod(hue + (double)251/360, 1);
		lightness -= 0.22;
		if (lightness < 0.2) {
			lightness += 0.6;
		}
	}

}

Vec<uchar, 3> ColorScale::getGrayScale(double scale) {
	int i = min(max((int)(scale * scaleLength), 0), scaleLength - 1);
	return grayTable[i];
}

Vec<uchar, 3> ColorScale::getHeatScale(double scale) {
	int i = min(max((int)(scale * scaleLength), 0), scaleLength - 1);
	return heatTable[i];
}

Vec<uchar, 3> ColorScale::getRainbowScale(double scale) {
	int i = min(max((int)(scale * scaleLength), 0), scaleLength - 1);
	return rainbowTable[i];
}

Vec<uchar, 3> ColorScale::getLabelColor(int label) {
	if (label < 0) {
		return Vec<uchar, 3>(0x80, 0x80, 0x80);
	} else {
		label = label % labelLength;
		return labelTable[label];
	}
}
