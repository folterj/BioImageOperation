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
	int clusterLabel = -1;

	double x = 0;
	double y = 0;
	double estimateX = 0;
	double estimateY = 0;
	double area = 0;
	double meanArea = 0;
	double rad = 0;
	double lengthMajor = 0;
	double lengthMinor = 0;
	double meanLengthMajor = 0;
	double meanLengthMinor = 0;
	double angle = 0;
	double orientation = 0;
	double lastMatchFactor = 1;

	double originX = 0;
	double originY = 0;
	double dx = 0;
	double dy = 0;
	double dist = 0;
	double totdist = 0;
	double lastClusterRad = 0;
	double forwardDist = 0;

	bool probation = true;
	bool isNew = true;
	bool assigned = true;
	bool isMerged = false;
	int activeCount = 0;
	int inactiveCount = 0;
	int lifeTime = 0;

	int minActive = 1;
	int maxInactive = 0;
	double fps = 0;
	double pixelSize = 1;
	double windowSize = 1;
	
	PathNode* lastPathNode = nullptr;

	vector<Point2d> points;
	vector<double> angles;


	Track(int minActive, int maxInactive, double fps, double pixelSize, double windowSize);
	void update(Cluster* cluster, double maxArea, double maxMoveDistance, bool trackParamsFinalised);
	void updateInactive();
	double getDistFromOrigin();

	void unAssign();
	void assign(double matchFactor = 0);
	bool checkLive(int* newLabel);
	bool isActive(bool needAssigned = false);
	double activeFactor();

	void draw(Mat* image, int drawMode, int ntracks);
	void drawPoint(Mat* image, Scalar color);
	void drawCircle(Mat* image, Scalar color);
	void drawEllipse(Mat* image, Scalar color);
	void drawBox(Mat* image, Scalar color);
	void drawAngle(Mat* image, Scalar color);
	void drawTracks(Mat* image, Scalar color, int ntracks = 1);
	void drawLabel(Mat* image, Scalar color, int drawMode);
	static string getCsvHeader(bool outputContour = false);
	string getCsv(bool outputContour = false, Cluster* cluster = nullptr);
	string toString();
};
