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
#include "Track.h"
#include "Constants.h"

using namespace cv;


class Track;		// forward declaration


/*
 * Cluster element
 */

class Cluster
{
public:
	vector<Track*> assignedTracks;

	double x = 0;
	double y = 0;
	double area = 0;
	double rad = 0;
	double length_major = 0;
	double length_minor = 0;
	double angle = 0;
	int clusterLabel = 0;

	Rect box;
	Moments moments;
	Mat clusterImage;

	double pixelSize = 1;


	Cluster(int clusterLabel, double x, double y, double area, Rect box, Moments* moments, Mat* clusterImage, double pixelSize);
	bool isAssignable(double trackedArea);
	void assign(Track* track);
	bool isAssigned();
	void unAssign(Track* track);
	void unAssign();

	double calcDistance(Track* track);
	double calcAreaDif(Track* track);
	double calcLengthDif(Track* track);
	double calcAngleDif(Track* track);
	double getRangeFactor(Track* track, double distance, double maxMoveDistance);
	double calcAreaFactor(Track* track, double areaDif);
	double calcLengthFactor(Track* track, double lengthDif);
	double calcAngleFactor(Track* track, double angleDif);

	bool hasSingleLabel();
	int getInitialLabel();
	string getLabels();
	void draw(Mat* image, int drawMode);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawFill(Mat* image, Scalar color);
	void drawLabel(Mat* image, Scalar color, int drawMode);
	static string getCsvHeader(bool writeContour = false);
	string getCsv(bool writeContour = false);
	vector<Point> getContour();
	string toString();
};
