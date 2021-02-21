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


 // https://www.learnopencv.com/blob-detection-using-opencv-python-c/

Cluster::Cluster(int clusterLabel, double x, double y, double area, Rect box, Moments* moments, Mat* clusterImage, double pixelSize) {
	this->clusterLabel = clusterLabel;
	this->area = area;
	this->x = x;
	this->y = y;
	this->box = box;
	this->moments = *moments;
	this->clusterImage = *clusterImage;
	this->pixelSize = pixelSize;
	if (moments->m00 != 0) {
		angle = Util::calcMomentsAngle(moments);
		rad = Util::calcMomentsMajorRadius(moments);
		lengthMajor = 2 * rad;
		lengthMinor = 2 * Util::calcMomentsMinorRadius(moments);
	}
	else {
		rad = sqrt(area);
	}
}

// * deprecated
bool Cluster::isAssignable(Track* track) {
	int n = (int)assignedTracks.size();
	double totalArea;
	bool isActive = track->isActive(false);

	if ((n > 0 && !isActive) || n >= Constants::maxMergedBlobs) {
		// for multiple matches: only allow active tracks
		return false;
	}

	totalArea = track->meanArea;
	for (Track* track0 : assignedTracks) {
		if (track0) {
			totalArea += track0->meanArea;
		}
	}

	// allow margin for (total) size
	if (n == 0) {
		return (totalArea * 0.5 < area);
	} else {
		return (totalArea * 0.75 < area);
	}
}

bool Cluster::isAssignable(int ntotal, double totalArea) {
	if (ntotal >= Constants::maxMergedBlobs) {
		return false;
	}

	if (ntotal == 1) {
		return true;
	} else {
		return (totalArea * 0.75 < area);
	}
}

bool Cluster::isAssignable(Track* track, int ntotal, double totalArea) {
	bool isActive = track->isActive(false);

	if ((ntotal > 1 && !isActive) || ntotal >= Constants::maxMergedBlobs) {
		// for multiple matches: only allow active tracks
		return false;
	}

	// allow margin for (total) size
	if (ntotal == 1) {
		return (totalArea * 0.5 < area);
	} else {
		return (totalArea * 0.75 < area);
	}
}

void Cluster::assign(Track* track) {
	assignedTracks.push_back(track);
}

bool Cluster::isAssigned() {
	return (assignedTracks.size() != 0);
}

bool Cluster::hasSingleTrack() {
	return (assignedTracks.size() == 1);
}

bool Cluster::isMerged() {
	return (assignedTracks.size() > 1);
}

bool Cluster::isSuspectMerged(Track* track) {
	return (area > 1.5 * track->area);
}

void Cluster::unAssign(Track* track) {
	auto position = find(assignedTracks.begin(), assignedTracks.end(), track);
	if (position != assignedTracks.end()) {
		assignedTracks.erase(position);
	}
}

void Cluster::unAssign() {
	assignedTracks.clear();
}

double Cluster::calcDistance(Track* track) {
	// project on line between previous and estimated new position
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

double Cluster::calcAreaDif(Track* track) {
	return abs(area - track->meanArea);
}

double Cluster::calcLengthDif(Track* track) {
	return abs(lengthMajor - track->meanLengthMajor);
}

double Cluster::calcAngleDif(Track* track) {
	return Util::calcShortAngleDif(angle, track->angle);
}

double Cluster::getRangeFactor(Track* track, double distance, double maxMoveDistance) {
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

double Cluster::calcAreaFactor(Track* track, double areaDif) {
	double areaFactor = 1;
	double a = track->meanArea;
	if (a == 0) {
		a = area;
	}
	if (a != 0) {
		areaFactor = 1 - areaDif / a;
		if (areaFactor < 0) {
			areaFactor = 0;
		}
	}
	return areaFactor;
}

double Cluster::calcLengthFactor(Track* track, double lengthDif) {
	double lengthFactor = 1;
	double len = track->meanLengthMajor;
	if (len == 0) {
		len = lengthMajor;
	}
	if (len != 0) {
		lengthFactor = 1 - lengthDif / len;
		if (lengthFactor < 0) {
			lengthFactor = 0;
		}
	}
	return lengthFactor;
}

double Cluster::calcAngleFactor(Track* track, double angleDif) {
	double angleFactor = 1;
	angleFactor = 1 - abs(angleDif) / 360;
	return angleFactor;
}

int Cluster::getInitialLabel() {
	int label = -1;

	if (!assignedTracks.empty()) {
		label = assignedTracks[0]->label;
	} else {
		label = clusterLabel;
	}
	return label;
}

string Cluster::getLabels() {
	string labels = "";

	for (Track* track : assignedTracks) {
		if (labels != "") {
			labels += ",";
		}
		labels += to_string(track->label);
	}
	return labels;
}

void Cluster::draw(Mat* image, int drawMode) {
	int label = -1;
	if (hasSingleTrack()) {
		label = getInitialLabel();
	}
	Scalar color = ColorScale::getLabelColor(label);
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
	int rad2 = (int)ceil(lengthMinor / 4);
	circle(*image, point, rad2, color, LineTypes::FILLED, LineTypes::LINE_AA);
}

void Cluster::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	circle(*image, point, (int)ceil(rad), color, 1, LineTypes::LINE_AA);
}

void Cluster::drawBox(Mat* image, Scalar color) {
	int rad2 = (int)ceil(rad);
	Rect rect((int)(x - rad2), (int)(y - rad2), (int)(rad2 * 2), (int)(rad2 * 2));
	rectangle(*image, rect, color, 1, LineTypes::LINE_AA);
}

void Cluster::drawAngle(Mat* image, Scalar color) {
	Util::drawAngle(image, x, y, rad, angle, color, false);
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
	vector<string> texts;
	HersheyFonts fontFace = HersheyFonts::FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	Point point((int)(x + rad), (int)(y + rad));
	Size size;
	string text;

	if ((drawMode & (int)ClusterDrawMode::Label) != 0) {
		text = getLabels();
		texts.push_back(text);
	}
	if ((drawMode & (int)ClusterDrawMode::LabelArea) != 0) {
		text = Util::format("%.0f", area);
		texts.push_back(text);
	}
	if ((drawMode & (int)ClusterDrawMode::LabelLength) != 0) {
		text = Util::format("%.1f", lengthMajor);
		texts.push_back(text);
	}
	if ((drawMode & (int)ClusterDrawMode::LabelAngle) != 0) {
		text = Util::format("%.0f", angle);
		texts.push_back(text);
	}

	for (string text : texts) {
		size = Util::drawText(image, text, point, fontFace, fontScale, color);
		point.y += (int)(size.height * 1.5);
	}
}

string Cluster::getCsvHeader(bool outputShapeFeatures, bool outputContour) {
	string header = "track_label,cluster_label,is_merged"
		",x,y"
		",angle"
		",area,rad,length_major,length_minor";
	if (outputShapeFeatures) {
		header += ",size_ratio,ellipsity,circularity,convexity";
	}
	if (outputContour) {
		header += ",contour";
	}
	return header;
}

string Cluster::getCsv(bool outputShapeFeatures, bool outputContour) {
	string csv;
	vector<Point> contour;
    
	csv = Util::replace(getLabels(), ",", " ") + "," + to_string(clusterLabel);
	csv += format(",%s", isMerged() ? "true" : "false");
	csv += format(",%f,%f,%f", x * pixelSize, y * pixelSize, angle);
	csv += format(",%f,%f,%f,%f", area * pixelSize * pixelSize, rad * pixelSize, lengthMajor * pixelSize, lengthMinor * pixelSize);

	if (outputShapeFeatures || outputContour) {
		contour = getContour();
	}
	if (outputShapeFeatures) {
		csv += Util::getShapeFeatures(&contour, area, lengthMajor, lengthMinor);
	}
	if (outputContour) {
		csv += ",";
		for (Point point : contour) {
			if (pixelSize == 1) {
				csv += Util::format("%d %d ", point.x, point.y);
			} else {
				csv += Util::format("%f %f ", point.x * pixelSize, point.y * pixelSize);
			}
		}
	}
	return csv;
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

string Cluster::toString() {
	return Util::format("#%d Labels:%s Area:%.0f Radius:%.0f Angle:%.0f X:%.0f Y:%.0f", clusterLabel, getLabels().c_str(), area, rad, angle, x, y);
}
