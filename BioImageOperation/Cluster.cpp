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

#include "Cluster.h"
#include "Constants.h"
#include "Util.h"


Cluster::Cluster(double x, double y, double area, double angle, Rect box, Mat* clusterImage)
{
	this->area = area;
	this->x = x;
	this->y = y;
	this->angle = angle;
	this->box = box;
	this->clusterImage = *clusterImage;
	rad = sqrt(area);
}

Cluster::Cluster(Moments* cvmoments, double area)
{
	this->area = area;
	x = cvmoments->m10 / area;
	y = cvmoments->m01 / area;
	angle = Util::getMomentsAngle(cvmoments);
	rad = sqrt(area);
}

void Cluster::unAssign()
{
	assignedTracks.clear();
}

bool Cluster::isAssignable(double trackedArea)
{
	int n = (int)assignedTracks.size();
	double totalArea;

	if (n == 0)
	{
		return true;
	}

	totalArea = trackedArea;
	for (ClusterTrack* track : assignedTracks)
	{
		totalArea += track->area;
	}
	return (totalArea * 0.75 < area && n < Constants::maxMergedBlobs);
}

void Cluster::assign(ClusterTrack* track)
{
	assignedTracks.push_back(track);
}

bool Cluster::isAssigned()
{
	return (assignedTracks.size() != 0);
}

double Cluster::calcDistance(ClusterTrack* track)
{
	return Util::calcDistance(track->estimateX, track->estimateY, x, y);
}

bool Cluster::isOverlap(ClusterTrack* track)
{
	return (calcDistance(track) - (2 * rad - track->rad) <= 0);
}

bool Cluster::inRange(ClusterTrack* track, double dist, double maxMoveDistance)
{
	return (maxMoveDistance <= 0 || dist - (2 * rad - track->rad) <= maxMoveDistance);
}

int Cluster::getLabel()
{
	int label0 = 0;

	if (assignedTracks.size() == 1)
	{
		label0 = assignedTracks[0]->label;
	}
	else if (assignedTracks.size() > 1)
	{
		label0 = 0x10000;
	}
	return label0;
}

int Cluster::getFirstLabel()
{
	int label0 = -1;

	if (!assignedTracks.empty())
	{
		label0 = assignedTracks[0]->label;
	}
	return label0;
}

System::String^ Cluster::getLabels()
{
	System::String^ labels = "";

	for (ClusterTrack* track : assignedTracks)
	{
		if (labels != "")
		{
			labels += ",";
		}
		labels += track->label;
	}
	return labels;
}

void Cluster::draw(Mat* image, ClusterDrawMode drawMode)
{
	Scalar color = Util::getLabelColor(getLabel());
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
	if ((drawMode & ClusterDrawMode::Fill) != ClusterDrawMode::None)
	{
		drawFill(image, color);
	}
}

void Cluster::drawPoint(Mat* image, Scalar color)
{
	cv::Point point((int)x, (int)y);

	drawMarker(*image, point, color, MARKER_CROSS, 2, 1, LINE_AA);
}

void Cluster::drawCircle(Mat* image, Scalar color)
{
	cv::Point point((int)x, (int)y);

	circle(*image, point, (int)rad, color, 1, LINE_AA);
}

void Cluster::drawBox(Mat* image, Scalar color)
{
	Rect rect((int)(x - rad), (int)(y - rad), (int)(rad * 2), (int)(rad * 2));

	rectangle(*image, rect, color, 1, LINE_AA);
}

void Cluster::drawAngle(Mat* image, Scalar color)
{
	int x0 = (int)(x - rad * cos(angle));
	int y0 = (int)(y - rad * sin(angle));
	int x1 = (int)(x + rad * cos(angle));
	int y1 = (int)(y + rad * sin(angle));

	line(*image, cv::Point(x0, y0), cv::Point(x1, y1), color, 1, LINE_AA);
}

void Cluster::drawLabel(Mat* image, Scalar color, bool showCount)
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
		labelx = getLabels();
	}
	
	putText(*image, Util::stdString(labelx), point, HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.5, color, 1, LINE_AA);
}

void Cluster::drawFill(Mat* image, Scalar color)
{
	if (Util::isValidImage(&clusterImage))
	{
		Mat clusterImage2(clusterImage.size(), image->type());
		clusterImage2.setTo(0);
		clusterImage2.setTo(color, clusterImage);			// use image as mask to convert to color

		clusterImage2.copyTo((*image)(box), clusterImage);	// use mask again; don't copy black pixels
	}
}

std::vector<Point> Cluster::getContour()
{
	std::vector<Point> contour;
	std::vector<std::vector<Point>> contours;
	findContours(clusterImage, contours, RetrievalModes::RETR_EXTERNAL, ContourApproximationModes::CHAIN_APPROX_NONE);
	for (Point point : contours[0])
	{
		contour.push_back(box.tl() + point);
	}
	return contour;
}

System::String^ Cluster::getCsv(bool writeContour)
{
	System::String^ s = System::String::Format("{0},{1},{2},{3},{4},{5}", getFirstLabel(), area, rad, angle, x, y);
	if (writeContour)
	{
		s += ",";
		for (Point point : getContour())
		{
			s += System::String::Format("{0} {1} ", point.x, point.y);
		}
	}
	return s;
}

System::String^ Cluster::ToString()
{
	return System::String::Format("Label:{0} Area:{1:F1} Radius:{2:F1} Angle:{3:F1} X:{4:F1} Y:{5:F1}", getLabels(), area, rad, angle, x, y);
}
