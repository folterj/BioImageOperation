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

using namespace std;
using namespace cv;


/*
 * Link between path nodes
 */

class PathLink
{
public:
	int label = 0;

	double x1, y1;
	double x2, y2;

	int created = 0;
	int count = 0;
	int totalUse = 0;
	int nNormal = 0;
	int nReverse = 0;
	double animPos = 0;
	bool used = false;

	PathLink(int label, double x1, double y1, double x2, double y2, int time);
	void updateUse(int time, bool reversed=false);
	int getMax();
	double getMaxCount(int time);
	double getUsage(int time);
	double getDirectionRate();
	void draw(Mat* image, Scalar color, int max, bool animate, int scale=1);
	string toString();
};
