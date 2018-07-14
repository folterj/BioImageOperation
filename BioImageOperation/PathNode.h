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

#pragma once
#include <vector>
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "Constants.h"

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

	std::vector<int> usage;
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
