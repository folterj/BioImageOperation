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
#include "ClusterTrack.h"
#include "Constants.h"

using namespace cv;


class ClusterTrack;		// forward declaration


/*
 * Cluster element
 */

class Cluster
{
public:
	std::vector<ClusterTrack*> assignedTracks;
	Moments moments;
	Mat clusterImage;

	double x = 0;
	double y = 0;

	Rect box;

	double area = 0;
	double angle = 0;

	double rad = 0;

	Cluster(double x, double y, double area, double angle, Rect box, Moments* moments, Mat* clusterImage);
	void unAssign();
	bool isAssignable(double trackedArea);
	void assign(ClusterTrack* track);
	bool isAssigned();

	double calcDistance(ClusterTrack* track);
	double calcAreaDif(ClusterTrack* track);
	bool isOverlap(ClusterTrack* track);
	double getRangeFactor(ClusterTrack* track, double distance, double maxMoveDistance);

	int getLabel();
	int getFirstLabel();
	string getLabels();
	void draw(Mat* image, int drawMode);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawFill(Mat* image, Scalar color);
	std::vector<Point> getContour();
	string getCsv(bool writeContour = false);
	void drawLabel(Mat* image, Scalar color, bool showCount);
	string toString();
};
