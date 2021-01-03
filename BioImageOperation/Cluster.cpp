/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#include "Cluster.h"
#include "Constants.h"
#include "Util.h"


Cluster::Cluster(double x, double y, double area, Rect box, Moments* moments, Mat* clusterImage) {
	this->area = area;
	this->x = x;
	this->y = y;
	this->angle = angle;
	this->box = box;
	this->moments = *moments;
	this->clusterImage = *clusterImage;
	if (moments->m00 != 0) {
		angle = Util::calcMomentsAngle(moments);
		rad = Util::calcMomentsMajorRadius(moments);
	} else {
		rad = sqrt(area);
	}
}

bool Cluster::isAssignable(double trackedArea) {
	int n = (int)assignedTracks.size();
	double totalArea;

	if (n == 0) {
		return true;
	}

	totalArea = trackedArea;
	for (ClusterTrack* track : assignedTracks) {
		if (track) {
			totalArea += track->area;
		}
	}
	return (totalArea * 0.75 < area && n < Constants::maxMergedBlobs);
}

void Cluster::assign(ClusterTrack* track) {
	assignedTracks.push_back(track);
}

bool Cluster::isAssigned() {
	return (assignedTracks.size() != 0);
}

void Cluster::unAssign(ClusterTrack* track) {
	auto position = find(assignedTracks.begin(), assignedTracks.end(), track);
	if (position != assignedTracks.end()) {
		assignedTracks.erase(position);
	}
}

void Cluster::unAssign() {
	assignedTracks.clear();
}

double Cluster::calcDistance(ClusterTrack* track) {
	double distance, distance1, distance2;
	double vx1 = track->x;
	double vy1 = track->y;
	double vx2 = track->estimateX;
	double vy2 = track->estimateY;
	double dvx = vx2 - vx1;
	double dvy = vy2 - vy1;

	if (dvx * dvx + dvy * dvy != 0) {
		double t = -((vx1 - x) * dvx + (vy1 - y) * dvy) / (dvx * dvx + dvy * dvy);
		if (t >= 0 && t <= 1) {
			distance = abs(dvx * (vy1 - y) - dvy * (vx1 - x)) / sqrt(dvx * dvx + dvy * dvy);
			return distance;
		}
	}

	distance1 = Util::calcDistance(track->x, track->y, x, y);
	distance2 = Util::calcDistance(track->estimateX, track->estimateY, x, y);
	distance = min(distance1, distance2);
	return distance;
}

double Cluster::calcAreaDif(ClusterTrack* track) {
	return abs(area - track->area);
}

double Cluster::calcAngleDif(ClusterTrack* track) {
	return Util::calcShortAngleDif(angle, track->angle);
}

double Cluster::getRangeFactor(ClusterTrack* track, double distance, double maxMoveDistance) {
	double rangeFactor = 1;
	double clusterRad, extraDist;

	if (maxMoveDistance != 0) {
		clusterRad = max(track->lastClusterRad, rad);
		//extraDist = abs(clusterRad - track->rad) * 2;
		extraDist = abs(2 * clusterRad - track->rad);
		rangeFactor = 1 - distance / (maxMoveDistance + extraDist);
	}
	return rangeFactor;
}

double Cluster::calcAreaFactor(ClusterTrack* track, double areaDif) {
	double areaFactor = 1;
	double a = max(track->area, area);
	if (!track->isMerged && a != 0) {
		areaFactor = 1 - pow(areaDif / a, 2);
	}
	return areaFactor;
}

double Cluster::calcAngleFactor(ClusterTrack* track, double angleDif) {
	double angleFactor = 1 - sqrt(abs(angleDif) / 360);
	return angleFactor;
}

int Cluster::getLabel() {
	int label0 = 0;

	if (assignedTracks.size() == 1) {
		label0 = assignedTracks[0]->label;
	} else if (assignedTracks.size() > 1) {
		label0 = 0x10000;
	}
	return label0;
}

int Cluster::getFirstLabel() {
	int label0 = -1;

	if (!assignedTracks.empty()) {
		label0 = assignedTracks[0]->label;
	}
	return label0;
}

string Cluster::getLabels() {
	string labels = "";

	for (ClusterTrack* track : assignedTracks) {
		if (labels != "") {
			labels += ",";
		}
		labels += to_string(track->label);
	}
	return labels;
}

void Cluster::draw(Mat* image, int drawMode) {
	Scalar color = Util::getLabelColor(getLabel());
	Scalar labelColor = Scalar(0x80, 0x80, 0x80);

	if ((drawMode & (int)ClusterDrawMode::Fill) != 0) {
		drawFill(image, color);
	}
	if ((drawMode & (int)ClusterDrawMode::Point) != 0) {
		drawPoint(image, color);
	}
	if ((drawMode & (int)ClusterDrawMode::Circle) != 0) {
		drawCircle(image, color);
	}
	if ((drawMode & (int)ClusterDrawMode::Box) != 0) {
		drawBox(image, color);
	}
	if ((drawMode & (int)ClusterDrawMode::Angle) != 0) {
		drawAngle(image, color);
	}
	drawLabel(image, labelColor, drawMode);
}

void Cluster::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	int thickness = rad / 4;
	if (thickness < 1) {
		thickness = 1;
	}
	drawMarker(*image, point, color, MarkerTypes::MARKER_CROSS, 1, thickness, LineTypes::LINE_AA);
}

void Cluster::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LineTypes::LINE_AA);
}

void Cluster::drawBox(Mat* image, Scalar color) {
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LineTypes::LINE_AA);
}

void Cluster::drawAngle(Mat* image, Scalar color) {
	double radAngle = Util::degreesToRadians(angle);
	int x0 = (int)(x - rad * cos(radAngle));
	int y0 = (int)(y - rad * sin(radAngle));
	int x1 = (int)(x + rad * cos(radAngle));
	int y1 = (int)(y + rad * sin(radAngle));

	line(*image, Point(x0, y0), Point(x1, y1), color, 1, LineTypes::LINE_AA);
}

void Cluster::drawFill(Mat* image, Scalar color) {
	if (Util::isValidImage(&clusterImage)) {
		Mat clusterImage2(clusterImage.size(), image->type());
		clusterImage2.setTo(0);
		clusterImage2.setTo(color, clusterImage);			// use image as mask to convert to color

		clusterImage2.copyTo((*image)(box), clusterImage);	// use mask again; don't copy black pixels
	}
}

void Cluster::drawLabel(Mat* image, Scalar color, int drawMode) {
	HersheyFonts fontFace = HersheyFonts::FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	Point point((int)(x + rad), (int)(y + rad));
	Size size;

	if ((drawMode & (int)ClusterDrawMode::Label) != 0) {
		size = Util::drawText(image, getLabels(), point, fontFace, fontScale, color);
		point.y += size.height * 1.5;
	}
	if ((drawMode & (int)ClusterDrawMode::LabelArea) != 0) {
		size = Util::drawText(image, Util::format("%.0f", area), point, fontFace, fontScale, color);
		point.y += size.height * 1.5;
	}
	if ((drawMode & (int)ClusterDrawMode::LabelAngle) != 0) {
		size = Util::drawText(image, Util::format("%.0f", angle), point, fontFace, fontScale, color);
		point.y += size.height * 1.5;
	}
}

vector<Point> Cluster::getContour() {
	vector<Point> contour;
	vector<vector<Point>> contours;
	findContours(clusterImage, contours, RetrievalModes::RETR_EXTERNAL, ContourApproximationModes::CHAIN_APPROX_NONE);
	for (Point point : contours[0]) {
		contour.push_back(box.tl() + point);
	}
	return contour;
}

string Cluster::getCsv(bool writeContour) {
	// https://www.learnopencv.com/blob-detection-using-opencv-python-c/
	string s = format("%d,%f,%f,%f,%f,%f", getFirstLabel(), area, rad, angle, x, y);
	/*
	if (assignedTracks.size() == 1)
	{
		s += System::String::Format(",{0},{1},{2},{3},{4},{5},{6},{7},{8},{9}",
			moments.m00, moments.m01, moments.m10,
			moments.m11, moments.m02, moments.m20,
			moments.m21, moments.m12, moments.m30, moments.m03);

		s += System::String::Format(",{0},{1},{2},{3},{4},{5},{6}",
			moments.mu02, moments.mu03, moments.mu11,
			moments.mu12, moments.mu20, moments.mu21,
			moments.mu30);

		s += System::String::Format(",{0},{1},{2},{3},{4},{5},{6}",
			moments.nu02, moments.nu03, moments.nu11,
			moments.nu12, moments.nu20, moments.nu21,
			moments.nu30);
	}
	*/

	/*
	double mx = moments.m10 / moments.m00;
	double my = moments.m01 / moments.m00;

	double a = moments.m20 / moments.m00 - mx * mx;
	double b = 2 * (moments.m11 / moments.m00 - mx * my);
	double c = moments.m02 / moments.m00 - my * my;

	double l = Math::Sqrt(8 * (a + c + Math::Sqrt(b * b + (a - c) * (a - c)))) / 2;
	double w = Math::Sqrt(8 * (a + c - Math::Sqrt(b * b + (a - c) * (a - c)))) / 2;

	double norml = l / rad;
	double normw = w / rad;
	double ratio = w / l;

	std::vector<Point> contour = getContour();
	double perimeter = arcLength(contour, true);
	std::vector<Point> hull;
	std::vector<Point2f> contour2;

	convexHull(contour, hull);
	approxPolyDP(hull, contour2, 0.001, true);

	double circularity = area * 4 * Math::PI / (perimeter * perimeter);
	double ellipsity = area / (Math::PI * l * w);
	double convexity = area / contourArea(contour2);

	if (assignedTracks.size() == 1) {
		s += System::String::Format(",{0},{1},{2},{3},{4},{5},{6},{7},{8},{9}",
			area, rad, l, w, norml, normw, ratio, circularity, ellipsity, convexity);
	}
	*/

	if (writeContour) {
		s += ",";
		for (Point point : getContour()) {
			s += Util::format("%d %d ", point.x, point.y);
		}
	}
	return s;
}

string Cluster::toString() {
	return Util::format("Label:%s Area:%.1f Radius:%.1f Angle:%.1f X:%.1f Y:%.1f", getLabels().c_str(), area, rad, angle, x, y);
}
