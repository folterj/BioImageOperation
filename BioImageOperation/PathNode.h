/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "Constants.h"

using namespace std;
using namespace cv;


/*
 * Tracked cluster element
 */

class ClusterTrack;	// forward declaration

class PathNode
{
public:
	int label = 0;

	double x = 0;
	double y = 0;

	vector<int> usage;
	int age = 0;
	int accumUsage = 1;
	int lastUse = 1;

	PathNode(int label, ClusterTrack* clusterTrack);
	void updateUse(int pathAge);
	double getAccumUsage();
	double getAccumUsage2(int totalAge);
	double matchDistance(ClusterTrack* clusterTrack, double maxDistance);
	void draw(Mat* image, Scalar color);
};
