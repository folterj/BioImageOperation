/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "PathNode.h"
#include "Util.h"


PathNode::PathNode(int label, Track* track, int pathAge) {
	this->label = label;
	this->created = pathAge;

	x = track->x;
	y = track->y;
}

void PathNode::updateUse(int pathAge) {
	usage.push_back(pathAge);
	accumUsage++;
	lastUse = pathAge;
	totalUse += pathAge;
}

float PathNode::getAccumUsage(int totalAge) {
	return (float)accumUsage / totalAge;
}

float PathNode::getAccumUsage2(int totalAge) {
	float totalUse = 0;

	for (int use : usage) {
		totalUse += 1.0 / (totalAge - use);
	}
	return totalUse;
}

float PathNode::getAccumUsage3(int totalAge) {
	return (float)totalUse / (totalAge + 1);
}

double PathNode::matchDistance(Track* track, double maxDistance) {
	double distance = Util::calcDistance(x, y, track->x, track->y);

	if (distance < maxDistance) {
		return distance;
	}
	return -1;
}

void PathNode::draw(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	drawMarker(*image, point, color, MarkerTypes::MARKER_CROSS, 2, 1, LineTypes::LINE_AA);
}

string PathNode::toString() {
	return Util::format("%d created:%d accumUsage:%d lastUse:%d X:%.0f Y:%.0f", label, created, accumUsage, lastUse, x, y);
}
