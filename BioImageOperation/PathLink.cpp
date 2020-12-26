/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "PathLink.h"


PathLink::PathLink() {
}

PathLink::PathLink(PathNode* node1, PathNode* node2) {
	this->node1 = node1;
	this->node2 = node2;
	addMatch(true);
}

void PathLink::addMatch(bool normalDirection) {
	if (normalDirection) {
		nNormal++;
	} else {
		nReverse++;
	}
}

int PathLink::getMax() {
	return max(nNormal, nReverse);
}

double PathLink::getAccumUsage(bool normalDirection, int maxUsage) {
	if (normalDirection) {
		return (double)nNormal / maxUsage;
	} else {
		return (double)nReverse / maxUsage;
	}
}

void PathLink::draw(Mat* image, Scalar color, int maxUsage, bool animate) {
	int x1 = (int)node1->x;
	int y1 = (int)node1->y;
	int x2 = (int)node2->x;
	int y2 = (int)node2->y;
	Point point;
	double animPos2;

	if (animate) {
		if (nNormal >= nReverse) {
			animPos2 = animPos;
			animPos += (double)(nNormal - nReverse) / maxUsage;
		} else {
			animPos2 = 1 - animPos;
			animPos += (double)(nReverse - nNormal) / maxUsage;
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
