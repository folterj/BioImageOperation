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
#include "Cluster.h"
#include "Constants.h"

using namespace cv;


class Cluster;		// forward declaration

class PathNode;		// forward declaration


/*
 * Tracked cluster element
 */

class Track
{
public:
	int label = 0;

	double x = 0;
	double y = 0;
	double estimateX = 0;
	double estimateY = 0;
	double area = 0;
	double rad = 0;
	double length_major = 0;
	double length_minor = 0;
	double angle = 0;
	double orientation = 0;
	bool orientInvertAngle = false;

	double originX = 0;
	double originY = 0;
	double dx = 0;
	double dy = 0;
	double dist = 0;
	double totdist = 0;
	double lastClusterRad = 0;
	double forwardDist = 0;

	bool isNew = true;
	bool assigned = true;
	bool isMerged = false;
	int activeCount = 0;
	int inactiveCount = 0;

	double fps = 0;
	double pixelSize = 1;
	double windowSize = 1;
	
	PathNode* lastPathNode = nullptr;

	vector<Point2d> points;
	vector<double> angles;


	Track(int label, double fps, double pixelSize, double windowSize);
	void update(Cluster* cluster, double maxArea, double maxMoveDistance, bool positionPrediction);
	double getDistFromOrigin();

	void unAssign();
	void assign();
	bool isActive(int minActive);

	void draw(Mat* image, int drawMode, int ntracks);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawTracks(Mat* image, Scalar color, int ntracks = 1);
	void drawLabel(Mat* image, Scalar color, int drawMode);
	static string getCsvHeader(bool writeContour = false);
	string getCsv(Cluster* cluster, bool writeContour = false);
	string toString();
};
