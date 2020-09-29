/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "ClusterTrack.h"
#include "Util.h"

using namespace System;


ClusterTrack::ClusterTrack(int label)
{
	this->label = label;
}

void ClusterTrack::update(Cluster* cluster, double maxArea, double maxMoveDistance, bool positionPrediction)
{
	double lastOrientation = orientation;
	double orientationGuess;
	double angle, angleInv, angleDif, angleDifInv;
	double newx, newy;
	int ntracks = (int)cluster->assignedTracks.size();
	
	isMerged = (ntracks > 1);

	newx = cluster->x;
	newy = cluster->y;
	angle = cluster->angle;

	if (!isNew)
	{
		// not new; can use position history
		dx = newx - x;
		dy = newy - y;
		dist = Util::calcDistance(dx, dy);

		totdist += dist;
		avgdist = totdist / points.size();
		if (dist > maxdist)
		{
			maxdist = dist;
		}

		orientationGuess = Math::Atan2(dy, dx);
		angleInv = (cluster->angle + Math::PI);
		angleDif = Util::calcAngleDif(angle, orientationGuess);
		angleDifInv = Util::calcAngleDif(angleInv, orientationGuess);
		if (dist > 0.1 * maxMoveDistance)
		{
			// if significant step: orientation from angle
			orientInvertAngle = (Math::Abs(angleDifInv) < Math::Abs(angleDif));
		}
		if (orientInvertAngle)
		{
			orientation = angleInv;
		}
		else
		{
			orientation = angle;
		}

		if (dist < maxMoveDistance && positionPrediction)
		{
			// estimate new position using delta; only if reasonable
			estimateX = newx + dx;
			estimateY = newy + dy;
		}
		else
		{
			estimateX = newx;
			estimateY = newy;
		}
	}
	else
	{
		estimateX = newx;
		estimateY = newy;
		orientation = angle;
	}

	if ((ntracks == 1 || area == 0) && cluster->area < maxArea)
	{
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

void ClusterTrack::unAssign()
{
	assigned = false;
}

void ClusterTrack::assign()
{
	assigned = true;
}

bool ClusterTrack::isActive(int minActive)
{
	return (assigned && activeCount >= minActive && inactiveCount == 0);
}

void ClusterTrack::draw(Mat* image, ClusterDrawMode drawMode, int ntracks)
{
	Scalar color = Util::getLabelColor(label);
	Scalar labelColor = Scalar(0x80, 0x80, 0x80);

	if ((drawMode & ClusterDrawMode::Point) != ClusterDrawMode::None)
	{
		drawPoint(image, color);
	}
	if ((drawMode & ClusterDrawMode::Circle) != ClusterDrawMode::None)
	{
		drawCircle(image, color);
	}
	if ((drawMode & ClusterDrawMode::Box) != ClusterDrawMode::None)
	{
		drawBox(image, color);
	}
	if ((drawMode & ClusterDrawMode::Angle) != ClusterDrawMode::None)
	{
		drawAngle(image, color);
	}
	if ((drawMode & ClusterDrawMode::Label) != ClusterDrawMode::None)
	{
		drawLabel(image, labelColor, false);
	}
	if ((drawMode & ClusterDrawMode::Labeln) != ClusterDrawMode::None)
	{
		drawLabel(image, labelColor, true);
	}
	if ((drawMode & ClusterDrawMode::Tracks) != ClusterDrawMode::None)
	{
		if (ntracks == 0)
		{
			ntracks = 10;
		}
		drawTracks(image, color, ntracks);
	}
	if ((drawMode & ClusterDrawMode::Track) != ClusterDrawMode::None)
	{
		drawTracks(image, color, 1);
	}
}

void ClusterTrack::drawPoint(Mat* image, Scalar color)
{
	cv::Point point((int)x, (int)y);

	drawMarker(*image, point, color, MARKER_CROSS, 2, 1, LINE_AA);
}

void ClusterTrack::drawCircle(Mat* image, Scalar color)
{
	cv::Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LINE_AA);
}

void ClusterTrack::drawBox(Mat* image, Scalar color)
{
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LINE_AA);
}

void ClusterTrack::drawAngle(Mat* image, Scalar color)
{
	int x0 = (int)(x - rad * cos(orientation));
	int y0 = (int)(y - rad * sin(orientation));
	int x1 = (int)(x + 2 * rad * cos(orientation));
	int y1 = (int)(y + 2 * rad * sin(orientation));

	arrowedLine(*image, cv::Point(x0, y0), cv::Point(x1, y1), color, 1, LINE_AA);
}

void ClusterTrack::drawLabel(Mat* image, Scalar color, bool showCount)
{
	cv::Point point((int)x, (int)y);
	System::String^ labelx;

	if (showCount)
	{
		labelx = area.ToString();
		point.y = (int)(y + rad);
	}
	else
	{
		labelx = label.ToString();
	}

	putText(*image, Util::stdString(labelx), point, HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.5, color, 1, LINE_AA);
}

void ClusterTrack::drawTracks(Mat* image, Scalar color, int ntracks)
{
	cv::Point point0, point1;
	bool init = false;
	int n = 0;

	for (int i = (int)points.size() - 1; i >= 0 && n <= ntracks; i--)
	{
		point1.x = (int)points[i].x;
		point1.y = (int)points[i].y;
		if (init)
		{
			line(*image, point0, point1, color, 1, LINE_AA);
		}
		point0 = point1;
		init = true;
		n++;
	}
}

System::String^ ClusterTrack::getCsv(Cluster* cluster, bool writeContour)
{
	std::vector<std::vector<Point>> contours;
	Point absPoint;

	System::String^ s = System::String::Format("{0},{1},{2},{3},{4},{5}", label, area, rad, orientation, x, y);
	if (writeContour)
	{
		s += ",";
		if (cluster)
		{
			for (Point point : cluster->getContour())
			{
				s += System::String::Format("{0} {1} ", point.x, point.y);
			}
		}
	}
	return s;
}

System::String^ ClusterTrack::ToString()
{
	return System::String::Format("Label:{0} Area:{1:F1} Radius:{2:F1} Angle:{3:F1} X:{4:F1} Y:{5:F1}", label, area, rad, orientation, x, y);
}
