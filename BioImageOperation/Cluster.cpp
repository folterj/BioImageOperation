/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "Cluster.h"
#include "Constants.h"
#include "Util.h"


Cluster::Cluster(double x, double y, double area, double angle, Rect box, Moments* moments, Mat* clusterImage) {
	this->area = area;
	this->x = x;
	this->y = y;
	this->angle = angle;
	this->box = box;
	this->moments = *moments;
	this->clusterImage = *clusterImage;
	rad = sqrt(area);
}

void Cluster::unAssign() {
	assignedTracks.clear();
}

bool Cluster::isAssignable(double trackedArea) {
	int n = (int)assignedTracks.size();
	double totalArea;

	if (n == 0) {
		return true;
	}

	totalArea = trackedArea;
	for (ClusterTrack* track : assignedTracks) {
		totalArea += track->area;
	}
	return (totalArea * 0.75 < area && n < Constants::maxMergedBlobs);
}

void Cluster::assign(ClusterTrack* track) {
	assignedTracks.push_back(track);
}

bool Cluster::isAssigned() {
	return (assignedTracks.size() != 0);
}

double Cluster::calcDistance(ClusterTrack* track) {
	return Util::calcDistance(track->estimateX, track->estimateY, x, y);
}

double Cluster::calcAreaDif(ClusterTrack* track) {
	return abs(area - track->area);
}

bool Cluster::isOverlap(ClusterTrack* track) {
	return (calcDistance(track) - (2 * rad - track->rad) <= 0);
}

double Cluster::getRangeFactor(ClusterTrack* track, double distance, double maxMoveDistance) {
	double rangeFactor = 1;

	if (maxMoveDistance != 0) {
		if (track->isMerged) {
			// allow for greater move distance after being merged
			distance -= (2 * rad);
		} else {
			distance -= (2 * rad - track->rad);
		}
		if (distance < 0) {
			distance = 0;
		}
		rangeFactor = 1 - distance / maxMoveDistance;
	}
	return rangeFactor;
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
	if ((drawMode & (int)ClusterDrawMode::Label) != 0) {
		drawLabel(image, labelColor, false);
	}
	if ((drawMode & (int)ClusterDrawMode::Labeln) != 0) {
		drawLabel(image, labelColor, true);
	}
}

void Cluster::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	drawMarker(*image, point, color, MARKER_CROSS, 2, 1, LINE_AA);
}

void Cluster::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LINE_AA);
}

void Cluster::drawBox(Mat* image, Scalar color) {
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LINE_AA);
}

void Cluster::drawAngle(Mat* image, Scalar color) {
	int x0 = (int)(x - rad * cos(angle));
	int y0 = (int)(y - rad * sin(angle));
	int x1 = (int)(x + rad * cos(angle));
	int y1 = (int)(y + rad * sin(angle));

	line(*image, Point(x0, y0), Point(x1, y1), color, 1, LINE_AA);
}

void Cluster::drawLabel(Mat* image, Scalar color, bool showCount) {
	Point point((int)x, (int)y);
	string labelx;

	if (showCount) {
		labelx = Util::format("%.0f", area);
		point.y = (int)(y + rad);
	} else {
		labelx = getLabels();
	}

	putText(*image, labelx, point, HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.5, color, 1, LINE_AA);
}

void Cluster::drawFill(Mat* image, Scalar color) {
	if (Util::isValidImage(&clusterImage)) {
		Mat clusterImage2(clusterImage.size(), image->type());
		clusterImage2.setTo(0);
		clusterImage2.setTo(color, clusterImage);			// use image as mask to convert to color

		clusterImage2.copyTo((*image)(box), clusterImage);	// use mask again; don't copy black pixels
	}
}

std::vector<Point> Cluster::getContour() {
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
