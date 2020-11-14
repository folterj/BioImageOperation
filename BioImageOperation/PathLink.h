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
#include "PathNode.h"

using namespace cv;


/*
 * Link between path nodes
 */

class PathLink
{
public:
	PathNode* node1;
	PathNode* node2;
	int nNormal = 0;
	int nReverse = 0;
	double animPos = 0;

	PathLink();
	PathLink(PathNode* node1, PathNode* node2);
	void addMatch(bool normalDirection);
	int getMax();
	double getAccumUsage(bool normalDirection, int maxUsage);
	void draw(Mat* image, Scalar color, int maxUsage, bool animate);
};
