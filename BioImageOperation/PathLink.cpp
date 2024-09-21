/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "PathLink.h"
#include "Util.h"


PathLink::PathLink(int label, double x1, double y1, double x2, double y2, int time) {
	this->label = label;
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
	this->created = time;
}

void PathLink::updateUse(int time, bool reversed) {
	count++;
	totalUse += time;
	if (!used) {
		used = true;
	}
	if (reversed) {
		nReverse++;
	} else {
		nNormal++;
	}
}

int PathLink::getMax() {
	return max(nNormal, nReverse);
}

double PathLink::getMaxCount(int time) {
	return (double)getMax() / (time + 1);
}

double PathLink::getUsage(int time) {
	return (double)totalUse / (time + 1);
}

double PathLink::getDirectionRate() {
	return (double)(nNormal - nReverse) / getMax();
}

void PathLink::draw(Mat* image, Scalar color, int max, bool animate, int scale) {
	int x1 = (int)(this->x1 * scale);
	int y1 = (int)(this->y1 * scale);
	int x2 = (int)(this->x2 * scale);
	int y2 = (int)(this->y2 * scale);
	Point point;
	double animPos2;

	if (animate) {
		if (nNormal >= nReverse) {
			animPos2 = animPos;
			animPos += (double)(nNormal - nReverse) / max;
		} else {
			animPos2 = 1 - animPos;
			animPos += (double)(nReverse - nNormal) / max;
		}

		while (animPos >= 1) {
			animPos -= 1;
		}

		point.x = (int)(x1 + animPos2 * (x2 - x1));
		point.y = (int)(y1 + animPos2 * (y2 - y1));

		drawMarker(*image, point, color, MarkerTypes::MARKER_CROSS, 2, 1, LineTypes::LINE_AA);
	} else {
		line(*image, Point(x1, y1), Point(x2, y2), color, 1, LineTypes::LINE_AA);
	}
}

string PathLink::toString() {
	return Util::format("%d created:%d count:%d totalUse:%d X1:%d Y1:%d X2:%d Y2:%d", label, created, count, totalUse, x1, y1, x2, y2);
}
