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
	double orientationGuess, slowMoveDist;
	double angleInv, angleDif, angleDifInv;
	double newx, newy, dx0, dy0, dist0, dangle;
	int ntracks = (int)cluster->assignedTracks.size();
	bool wasMerged = isMerged;

	isMerged = (ntracks > 1);

	if (isMerged) {
		// move towards merged centre
		slowMoveDist = maxMoveDistance * 0.1;
		dx0 = cluster->x - x;
		dy0 = cluster->y - y;
		dist0 = Util::calcDistance(dx0, dy0);
		if (dist0 > cluster->rad * 0.5) {
			// track position should always stay inside merged cluster
			x += dx0 * 0.5;
			y += dy0 * 0.5;
		}
		if (dist0 > slowMoveDist) {
			dx0 *= slowMoveDist / dist0;
			dy0 *= slowMoveDist / dist0;
			dist0 = slowMoveDist;
		}
		if (dist > slowMoveDist) {
			dx *= slowMoveDist / dist;
			dy *= slowMoveDist / dist;
			dist = slowMoveDist;
		}
		// movement towards centre
		dx = (dx * 0.9 + dx0 * 0.1);
		dy = (dy * 0.9 + dy0 * 0.1);
		dist = Util::calcDistance(dx, dy);
		newx = x + dx;
		newy = y + dy;
		estimateX = newx + dx;
		estimateY = newy + dy;

		dangle = Util::calcShortAngleDif(angle, cluster->angle);
		angle += dangle * 0.01;
	} else if (isNew) {
		newx = cluster->x;
		newy = cluster->y;
		estimateX = newx;
		estimateY = newy;
	} else {
		// not new; can use position history
		newx = cluster->x;
		newy = cluster->y;
		dx = newx - x;
		dy = newy - y;
		dist = Util::calcDistance(dx, dy);

		totdist += dist;
		avgdist = totdist / points.size();
		if (dist > maxdist) {
			maxdist = dist;
		}

		if (abs(angle) > 45 && abs(cluster->angle) > 45 && angle * cluster->angle < 0) {
			// angle 'swapped sign'
			forwardDist = -forwardDist;
		}
		angle = cluster->angle;
		orientationGuess = Util::calcAngle(dy, dx);
		angleDif = Util::calcAngleDif(angle, orientationGuess);
		angleDifInv = Util::calcAngleDif(angle + 180, orientationGuess);
		if (abs(angleDif) < abs(angleDifInv)) {
			forwardDist += dist;
		} else {
			forwardDist -= dist;
		}

		if (dist < maxMoveDistance && positionPrediction) {
			// estimate new position using delta; only if reasonable
			estimateX = newx + dx;
			estimateY = newy + dy;
		} else {
			estimateX = newx;
			estimateY = newy;
		}
	}

	if (forwardDist >= 0) {
		orientation = angle;
	} else {
		orientation = angle + 180;
	}

	if ((ntracks == 1 || area == 0) && cluster->area < maxArea) {
		// only assign area if single track
		area = cluster->area;
		rad = cluster->rad;
	}

	x = newx;
	y = newy;

	points.push_back(Point2d(x, y));

	lastClusterRad = cluster->rad;
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
	if ((drawMode & (int)ClusterDrawMode::Tracks) != 0) {
		if (ntracks == 0) {
			ntracks = 10;
		}
		drawTracks(image, color, ntracks);
	}
	if ((drawMode & (int)ClusterDrawMode::Track) != 0) {
		drawTracks(image, color, 1);
	}
	drawLabel(image, labelColor, drawMode);
}

void ClusterTrack::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	int thickness = rad / 4;
	if (thickness < 1) {
		thickness = 1;
	}
	drawMarker(*image, point, color, MarkerTypes::MARKER_CROSS, 1, thickness, LineTypes::LINE_AA);
}

void ClusterTrack::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LineTypes::LINE_AA);
}

void ClusterTrack::drawBox(Mat* image, Scalar color) {
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LineTypes::LINE_AA);
}

void ClusterTrack::drawAngle(Mat* image, Scalar color) {
	double radOrientation = Util::degreesToRadians(orientation);
	int x0 = (int)(x - rad * cos(radOrientation));
	int y0 = (int)(y - rad * sin(radOrientation));
	int x1 = (int)(x + 2 * rad * cos(radOrientation));
	int y1 = (int)(y + 2 * rad * sin(radOrientation));

	arrowedLine(*image, Point(x0, y0), Point(x1, y1), color, 1, LineTypes::LINE_AA);
}

void ClusterTrack::drawTracks(Mat* image, Scalar color, int ntracks) {
	Point point0, point1;
	bool init = false;
	int n = 0;

	for (int i = (int)points.size() - 1; i >= 0 && n <= ntracks; i--) {
		point1.x = (int)points[i].x;
		point1.y = (int)points[i].y;
		if (init) {
			line(*image, point0, point1, color, 1, LineTypes::LINE_AA);
		}
		point0 = point1;
		init = true;
		n++;
	}
}

void ClusterTrack::drawLabel(Mat* image, Scalar color, int drawMode) {
	HersheyFonts fontFace = HersheyFonts::FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	Point point((int)(x + rad), (int)(y + rad));
	Size size;

	if ((drawMode & (int)ClusterDrawMode::Label) != 0) {
		size = Util::drawText(image, to_string(label), point, fontFace, fontScale, color);
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
