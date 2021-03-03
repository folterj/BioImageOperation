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


Track::Track(int label, int minActive, double fps, double pixelSize, double windowSize) {
	this->label = label;
	this->minActive = minActive;
	this->fps = fps;
	this->pixelSize = pixelSize;
	this->windowSize = windowSize;
}

void Track::update(Cluster* cluster, double maxArea, double maxMoveDistance, bool trackParamsFinalised) {
	double orientationGuess, slowMoveDist;
	double angleDif, angleDifInv;
	double newx, newy, dx0, dy0, dist0, dangle;
	int ntracks = (int)cluster->assignedTracks.size();

	clusterLabel = cluster->clusterLabel;

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
		angle = cluster->angle;
		estimateX = newx;
		estimateY = newy;
		originX = newx;
		originY = newy;
		if (x != 0 || y != 0) {
			dx = newx - x;
			dy = newy - y;
			dist = Util::calcDistance(dx, dy);
		}
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

		if (dist < maxMoveDistance) {
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
		orientation = Util::normAngle(angle + 180);
	}

	if ((ntracks == 1 && cluster->area < maxArea) || area == 0) {
		// only assign area if single track
		area = cluster->area;
		rad = cluster->rad;
		lengthMajor = cluster->lengthMajor;
		lengthMinor = cluster->lengthMinor;
	}

	x = newx;
	y = newy;

	if (isNew) {
		meanArea = area;
		meanLengthMajor = lengthMajor;
		meanLengthMinor = lengthMinor;
	} else {
		meanArea = meanArea * 0.9 + area * 0.1;
		meanLengthMajor = meanLengthMajor * 0.9 + lengthMajor * 0.1;
		meanLengthMinor = meanLengthMinor * 0.9 + lengthMinor * 0.1;
	}

	points.push_back(Point2d(x, y));
	angles.push_back(orientation);

	if (trackParamsFinalised) {
		lastClusterRad = cluster->rad;
		activeCount++;
		isNew = false;
	}
	inactiveCount = 0;
}

void Track::updateInactive() {
	if (activeCount > 0) {
		activeCount--;
	}
	inactiveCount++;
	clusterLabel = -1;
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

bool Track::isActive(bool needAssigned) {
	return ((!needAssigned || assigned) && activeCount >= minActive && inactiveCount == 0);
}

double Track::activeFactor() {
	double factor = (double)(activeCount + 1) / (minActive + 1);
	if (factor > 1) {
		factor = 1;
	}
	return factor;
}

void Track::draw(Mat* image, int drawMode, int ntracks) {
	Scalar color = ColorScale::getLabelColor(label);
	Scalar labelColor = Scalar(0x80, 0x80, 0x80);

	if (isActive()) {
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
	}
	drawLabel(image, labelColor, drawMode);
}

void Track::drawPoint(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	int rad2 = (int)ceil(meanLengthMinor / 4);
	circle(*image, point, rad2, color, LineTypes::FILLED, LineTypes::LINE_AA);
}

void Track::drawCircle(Mat* image, Scalar color) {
	Point point((int)x, (int)y);
	int rad2 = (int)ceil(meanLengthMajor / 2);
	if (rad2 == 0) {
		rad2 = rad;
	}
	circle(*image, point, rad2, color, 1, LineTypes::LINE_AA);
}

void Track::drawBox(Mat* image, Scalar color) {
	int rad2 = (int)ceil(meanLengthMajor / 2);
	if (rad2 == 0) {
		rad2 = rad;
	}
	Rect rect((int)(x - rad2), (int)(y - rad2), (int)(rad2 * 2), (int)(rad2 * 2));
	rectangle(*image, rect, color, 1, LineTypes::LINE_AA);
}

void Track::drawAngle(Mat* image, Scalar color) {
	double rad2 = meanLengthMajor / 2;
	if (rad2 == 0) {
		rad2 = rad;
	}
	Util::drawAngle(image, x, y, rad2, orientation, color, (forwardDist != 0));
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
	vector<string> texts;
	HersheyFonts fontFace = HersheyFonts::FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	Point point((int)(x + rad), (int)(y + rad));
	Size size;
	string text;

	if ((drawMode & (int)ClusterDrawMode::Label) != 0) {
		text = to_string(label);
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
		if (!isActive()) {
			text = "(" + text + ")";
		}
		size = Util::drawText(image, text, point, fontFace, fontScale, color);
		point.y += (int)(size.height * 1.5);
	}
}

string Track::getCsvHeader(bool outputShapeFeatures, bool outputContour) {
	string header = "track_label,cluster_label,is_merged"
					",x,y,v,projection,v_projection,a,dist_tot,dist_origin"
					",angle,v_angle,a_angle"
					",area,area1,length_major,length_major1,length_minor,length_minor1,rad";
	if (outputShapeFeatures) {
		header += ",size_ratio,ellipsity,circularity,convexity";
	}
	if (outputContour) {
		header += ",contour";
	}
	return header;
}

string Track::getCsv(bool outputShapeFeatures, bool outputContour, Cluster* cluster) {
	string csv;
	vector<Point> contour;
	Point2d* point;
	Point2d* lastPoint;
	double dist, dx, dy, dx1, dy1, lastDist, ddist, proj, centDist;
	double dangle, lastAngle, lastDangle, ddangle, radAngle;
	double v = 0;
	double v_angle = 0;
	double a = 0;
	double a_angle = 0;
	double vProjection = 0;
	double projection = 0;
	int vn = 0;
	int an = 0;
	int ii = 0;
	int n = (int)(round(fps * windowSize));

	csv = format("%d,", label);
	if (clusterLabel >= 0) {
		csv += to_string(clusterLabel);
	}
	csv += format(",%s", isMerged ? "true" : "false");

	for (int i = points.size() - 1; i >= 0; i--) {
		// reverse order loop: inverse delta subtractions
		point = &points[i];
		angle = angles[i];
		if (ii > 0) {
			dx = lastPoint->x - point->x;
			dy = lastPoint->y - point->y;
			dist = Util::calcDistance(dx, dy);
			dangle = Util::calcAngleDif(angle, lastAngle);
			radAngle = Util::degreesToRadians(angle);
			dx1 = cos(radAngle);
			dy1 = sin(radAngle);
			proj = (dx / dist) * dx1 + (dy / dist) * dy1;
			if (vn < n) {
				v += dist;
				v_angle += dangle;
				projection += proj;
				vProjection += dist * proj;
				vn++;
			}
			if (ii > 1) {
				ddist = lastDist - dist;
				ddangle = lastDangle - dangle;
				if (an < n) {
					a += ddist;
					a_angle += ddangle;
					an++;
				} else {
					break;			// completed a and v
				}
			}
			lastDist = dist;
			lastDangle = dangle;
		}
		lastPoint = point;
		lastAngle = angle;
		ii++;
	}

	if (vn != 0) {
		v /= vn;
		v_angle /= vn;
		projection /= vn;
		vProjection /= vn;
	}
	if (an != 0) {
		a /= an;
		a_angle /= an;
	}

	centDist = Util::calcDistance(originX, originY, x, y);

	csv += format(",%f,%f,%f,%f,%f,%f", x * pixelSize, y * pixelSize, v * pixelSize * fps, projection, vProjection * pixelSize * fps, a * pixelSize * fps * fps);
	csv += format(",%f,%f", totdist * pixelSize, centDist * pixelSize);
	csv += format(",%f,%f,%f", orientation, v_angle * fps, a_angle * fps * fps);
	csv += format(",%f,%f,%f,%f,%f,%f,%f",
					area * pixelSize * pixelSize, meanArea * pixelSize * pixelSize,
					lengthMajor * pixelSize, meanLengthMajor * pixelSize,
					lengthMinor * pixelSize, meanLengthMinor * pixelSize,
					rad * pixelSize);

	if (outputShapeFeatures || outputContour) {
		if (cluster) {
			contour = cluster->getContour();
		}
	}
	if (outputShapeFeatures) {
		if (cluster && cluster->hasSingleTrack()) {
			csv += Util::getShapeFeatures(&contour, area, lengthMajor, lengthMinor);
		} else {
			csv += ",,,,";
		}
	}
	if (outputContour) {
		csv += ",";
		if (cluster && cluster->hasSingleTrack()) {
			for (Point point : contour) {
				if (pixelSize == 1) {
					csv += Util::format("%d %d ", point.x, point.y);
				} else {
					csv += Util::format("%f %f ", point.x * pixelSize, point.y * pixelSize);
				}
			}
		}
	}
	return csv;
}

string Track::toString() {
	return Util::format("Label:%d Area:%.0f Radius:%.0f Orientation:%.0f X:%.0f Y:%.0f", label, area, rad, orientation, x, y);
}
