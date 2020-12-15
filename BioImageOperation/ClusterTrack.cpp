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
#include "ClusterTrack.h"
#include "Util.h"


ClusterTrack::ClusterTrack(int label) {
	this->label = label;
}

void ClusterTrack::update(Cluster* cluster, double maxArea, double maxMoveDistance, bool positionPrediction) {
	double lastOrientation = orientation;
	double orientationGuess;
	double angle, angleInv, angleDif, angleDifInv;
	double newx, newy;
	int ntracks = (int)cluster->assignedTracks.size();

	isMerged = (ntracks > 1);

	newx = cluster->x;
	newy = cluster->y;
	angle = cluster->angle;

	if (!isNew) {
		// not new; can use position history
		dx = newx - x;
		dy = newy - y;
		dist = Util::calcDistance(dx, dy);

		totdist += dist;
		avgdist = totdist / points.size();
		if (dist > maxdist) {
			maxdist = dist;
		}

		orientationGuess = atan2(dy, dx);
		angleInv = (cluster->angle + M_PI);
		angleDif = Util::calcAngleDif(angle, orientationGuess);
		angleDifInv = Util::calcAngleDif(angleInv, orientationGuess);
		if (dist > 0.1 * maxMoveDistance) {
			// if significant step: orientation from angle
			orientInvertAngle = (abs(angleDifInv) < abs(angleDif));
		}
		if (orientInvertAngle) {
			orientation = angleInv;
		} else {
			orientation = angle;
		}

		if (dist < maxMoveDistance && positionPrediction) {
			// estimate new position using delta; only if reasonable
			estimateX = newx + dx;
			estimateY = newy + dy;
		} else {
			estimateX = newx;
			estimateY = newy;
		}
	} else {
		estimateX = newx;
		estimateY = newy;
		orientation = angle;
	}

	if ((ntracks == 1 || area == 0) && cluster->area < maxArea) {
		// only assign area if single track
		area = cluster->area;
		rad = sqrt(area);
	}

	x = newx;
	y = newy;

	points.push_back(Point2d(x, y));

	isNew = false;
	activeCount++;
	inactiveCount = 0;
	assign();
}

void ClusterTrack::unAssign() {
	assigned = false;
}

void ClusterTrack::assign() {
	assigned = true;
}

bool ClusterTrack::isActive(int minActive) {
	return (assigned && activeCount >= minActive && inactiveCount == 0);
}

void ClusterTrack::draw(Mat* image, int drawMode, int ntracks) {
	Scalar color = Util::getLabelColor(label);
	Scalar labelColor = Scalar(0x80, 0x80, 0x80);

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
	if ((drawMode & (int)ClusterDrawMode::Tracks) != 0) {
		if (ntracks == 0) {
			ntracks = 10;
		}
		drawTracks(image, color, ntracks);
	}
	if ((drawMode & (int)ClusterDrawMode::Track) != 0) {
		drawTracks(image, color, 1);
	}
}

void ClusterTrack::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	drawMarker(*image, point, color, MARKER_CROSS, 2, 1, LINE_AA);
}

void ClusterTrack::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LINE_AA);
}

void ClusterTrack::drawBox(Mat* image, Scalar color) {
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LINE_AA);
}

void ClusterTrack::drawAngle(Mat* image, Scalar color) {
	int x0 = (int)(x - rad * cos(orientation));
	int y0 = (int)(y - rad * sin(orientation));
	int x1 = (int)(x + 2 * rad * cos(orientation));
	int y1 = (int)(y + 2 * rad * sin(orientation));

	arrowedLine(*image, Point(x0, y0), Point(x1, y1), color, 1, LINE_AA);
}

void ClusterTrack::drawLabel(Mat* image, Scalar color, bool showCount) {
	Point point((int)x, (int)y);
	string labelx;

	if (showCount) {
		labelx = Util::format("%.0f", area);
		point.y = (int)(y + rad);
	} else {
		labelx = to_string(label);
	}

	putText(*image, labelx, point, HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.5, color, 1, LINE_AA);
}

void ClusterTrack::drawTracks(Mat* image, Scalar color, int ntracks) {
	Point point0, point1;
	bool init = false;
	int n = 0;

	for (int i = (int)points.size() - 1; i >= 0 && n <= ntracks; i--) {
		point1.x = (int)points[i].x;
		point1.y = (int)points[i].y;
		if (init) {
			line(*image, point0, point1, color, 1, LINE_AA);
		}
		point0 = point1;
		init = true;
		n++;
	}
}

string ClusterTrack::getCsv(Cluster* cluster, bool writeContour) {
	vector<vector<Point>> contours;
	Point absPoint;

	string s = format("%d,%f,%f,%f,%f,%f", label, area, rad, orientation, x, y);
	if (writeContour) {
		s += ",";
		if (cluster) {
			if (cluster->assignedTracks.size() == 1) {
				for (Point point : cluster->getContour()) {
					s += Util::format("%d %d ", point.x, point.y);
				}
			}
		}
	}
	return s;
}

string ClusterTrack::toString() {
	return Util::format("Label:%d Area:%.1f Radius:%.1f Angle:%.1f X:%.1f Y:%.1f", label, area, rad, orientation, x, y);
}
