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
#include "ClusterTrack.h"
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "Constants.h"

using namespace System::Collections::Generic;
using namespace cv;


class ClusterTrack;	// forward declaration


/*
 * Cluster element
 */

class Cluster
{
public:
	std::vector<ClusterTrack*> assignedTracks;
	Mat clusterImage;

	double x = 0;
	double y = 0;

	Rect box;

	double area = 0;
	double angle = 0;

	double rad = 0;

	Cluster(double x, double y, double area, double angle, Rect box, Mat* clusterImage);
	void unAssign();
	bool isAssignable(double trackedArea);
	void assign(ClusterTrack* track);
	bool isAssigned();

	double calcDistance(ClusterTrack* track);
	double calcAreaDif(ClusterTrack* track);
	bool isOverlap(ClusterTrack* track);
	bool inRange(ClusterTrack* track, double distance, double maxMoveDistance);

	int getLabel();
	int getFirstLabel();
	System::String^ getLabels();
	void draw(Mat* image, ClusterDrawMode drawMode);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawFill(Mat* image, Scalar color);
	std::vector<Point> getContour();
	System::String^ getCsv(bool writeContour = false);
	void drawLabel(Mat* image, Scalar color, bool showCount);

	virtual System::String^ ToString();
};
