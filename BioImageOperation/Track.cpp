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
#include "Track.h"
#include "Util.h"


Track::Track(int label, double fps, double pixelSize, double windowSize) {
	this->label = label;
	this->fps = fps;
	this->pixelSize = pixelSize;
	this->windowSize = windowSize;
}

void Track::update(Cluster* cluster, double maxArea, double maxMoveDistance, bool positionPrediction) {
	double orientationGuess, slowMoveDist;
	double angleDif, angleDifInv;
	double newx, newy, dx0, dy0, dist0, dangle;
	int ntracks = (int)cluster->assignedTracks.size();

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
		dx = (dx * 0.9 + dx0 * 0.1) * 0.5;
		dy = (dy * 0.9 + dy0 * 0.1) * 0.5;
		dist = Util::calcDistance(dx, dy);
		newx = x + dx;
		newy = y + dy;
		estimateX = newx + dx;
		estimateY = newy + dy;

		forwardDist = 0;	// reset orientation when merged

		dangle = Util::calcShortAngleDif(angle, cluster->angle);
		angle += dangle * 0.01;
	} else if (isNew) {
		newx = cluster->x;
		newy = cluster->y;
		estimateX = newx;
		estimateY = newy;
		originX = newx;
		originY = newy;
	} else {
		// not new; can use position history
		newx = cluster->x;
		newy = cluster->y;
		dx = newx - x;
		dy = newy - y;
		dist = Util::calcDistance(dx, dy);
		totdist += dist;

		if (abs(angle) > 45 && abs(cluster->angle) > 45 && angle * cluster->angle < 0) {
			// angle has 'swapped sign'
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

	if ((ntracks == 1 && cluster->area < maxArea) || area == 0) {
		// only assign area if single track
		area = cluster->area;
		rad = cluster->rad;
		length_major = cluster->length_major;
		length_minor = cluster->length_minor;
	}

	x = newx;
	y = newy;

	points.push_back(Point2d(x, y));
	if (!isNew) {
		angles.push_back(orientation);
	}

	lastClusterRad = cluster->rad;
	isNew = false;
	activeCount++;
	inactiveCount = 0;
}

double Track::getDistFromOrigin() {
	return Util::calcDistance(originX, originY, x, y);
}

void Track::unAssign() {
	assigned = false;
}

void Track::assign() {
	assigned = true;
}

bool Track::isActive(int minActive) {
	return (assigned && activeCount >= minActive && inactiveCount == 0);
}

void Track::draw(Mat* image, int drawMode, int ntracks) {
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

void Track::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	int thickness = rad / 4;
	if (thickness < 1) {
		thickness = 1;
	}
	drawMarker(*image, point, color, MarkerTypes::MARKER_CROSS, 1, thickness, LineTypes::LINE_AA);
}

void Track::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LineTypes::LINE_AA);
}

void Track::drawBox(Mat* image, Scalar color) {
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LineTypes::LINE_AA);
}

void Track::drawAngle(Mat* image, Scalar color) {
	Util::drawAngle(image, x, y, rad, orientation, color, (forwardDist != 0));
}

void Track::drawTracks(Mat* image, Scalar color, int ntracks) {
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

void Track::drawLabel(Mat* image, Scalar color, int drawMode) {
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

string Track::getCsvHeader(bool writeContour) {
	string header = "label,x,y,v,a"
					",dist_tot,dist_origin"
					",angle,v_angle,a_angle"
					",area,rad,length_major,length_minor";
	if (writeContour) {
		header += ",contour";
	}
	return header;
}

string Track::getCsv(Cluster* cluster, bool writeContour) {
	string csv = format("%d", label);
	vector<vector<Point>> contours;
	Point2d lastpoint;
	double d, lastd, dd, lastangle, centdist;
	double v = 0;
	double v_angle = 0;
	double a = 0;
	double a_angle = 0;
	int vn = 0;
	int an = 0;
	int i = 0;
	int n = (int)(round(fps * windowSize));

	for (auto point = points.rbegin(); point != points.rend(); point++) {
		if (i > 0) {
			d = Util::calcDistance(lastpoint, *point);
			if (vn < n) {
				v += d;
				vn++;
			}
			if (i > 1) {
				dd = lastd - d;		// reverse order loop
				if (an < n) {
					a += dd;
					an++;
				} else {
					break;			// completed a and v
				}
			}
			lastd = d;
		}
		lastpoint = *point;
		i++;
	}
	if (vn != 0) {
		v = v / vn * pixelSize * fps;
	}
	if (an != 0) {
		a = a / an * pixelSize * fps * fps;
	}

	vn = 0;
	an = 0;
	i = 0;
	for (auto angle = angles.rbegin(); angle != angles.rend(); angle++) {
		if (i > 0) {
			d = lastangle - *angle;
			if (vn < n) {
				v_angle += d;
				vn++;
			}
			if (i > 1) {
				dd = lastd - d;		// reverse order loop
				if (an < n) {
					a_angle += dd;
					an++;
				} else {
					break;			// completed a and v
				}
			}
			lastd = d;
		}
		lastangle = *angle;
		i++;
	}
	if (vn != 0) {
		v_angle = v_angle / vn * fps;
	}
	if (an != 0) {
		a_angle = a_angle / an * fps * fps;
	}

	centdist = Util::calcDistance(originX, originY, x, y);

	csv += format(",%f,%f,%f,%f", x * pixelSize, y * pixelSize, v, a);
	csv += format(",%f,%f", totdist * pixelSize, centdist * pixelSize);
	csv += format(",%f,%f,%f", orientation, v_angle, a_angle);
	csv += format(",%f,%f,%f,%f", area * pixelSize * pixelSize, rad * pixelSize, length_major * pixelSize, length_minor * pixelSize);
	if (writeContour) {
		csv += ",";
		if (cluster) {
			if (cluster->hasSingleLabel()) {
				for (Point point : cluster->getContour()) {
					if (pixelSize == 1) {
						csv += Util::format("%d %d ", point.x, point.y);
					} else {
						csv += Util::format("%f %f ", point.x * pixelSize, point.y * pixelSize);
					}
				}
			}
		}
	}
	return csv;
}

string Track::toString() {
	return Util::format("Label:%d Area:%.0f Radius:%.0f Orientation:%.0f X:%.0f Y:%.0f", label, area, rad, orientation, x, y);
}
