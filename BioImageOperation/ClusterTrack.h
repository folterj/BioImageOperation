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
#include "Cluster.h"
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "Constants.h"

using namespace System::Collections::Generic; 
using namespace cv;


class Cluster;		// forward declaration

class PathNode;	// forward declaration


/*
 * Tracked cluster element
 */

class ClusterTrack
{
public:
	std::vector<Point2d> points;

	int label = 0;

	double x = 0;
	double y = 0;

	double orientation = 0;

	bool orientInvertAngle = false;
	double area = 0;
	double rad = 0;
	double dx = 0;
	double dy = 0;
	double dist = 0;
	double totdist = 0;
	double avgdist = 0;
	double maxdist = 0;

	double estimateX = 0;
	double estimateY = 0;

	bool isNew = true;
	bool assigned = true;
	int activeCount = 0;
	int inactiveCount = 0;
	
	PathNode* lastPathNode = NULL;

	ClusterTrack(int label);
	void update(Cluster* cluster, double maxArea, double maxMoveDistance, bool positionPrediction);
	bool preferEstimatePosition(Cluster* cluster);

	void unAssign();
	void assign();
	bool isActive(int minActive);

	void draw(Mat* image, ClusterDrawMode drawMode, int ntracks);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawLabel(Mat* image, Scalar color, bool showCount);
	void drawTracks(Mat* image, Scalar color, int ntracks = 1);
	System::String^ getCsv(Cluster* cluster, bool writeContour = false);
	virtual System::String^ ToString();
};
