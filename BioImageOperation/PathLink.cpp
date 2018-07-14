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

#include "PathLink.h"

using namespace System;


PathLink::PathLink()
{
}

PathLink::PathLink(PathNode* node1, PathNode* node2)
{
	this->node1 = node1;
	this->node2 = node2;
	addMatch(true);
}

void PathLink::addMatch(bool normalDirection)
{
	if (normalDirection)
	{
		nNormal++;
	}
	else
	{
		nReverse++;
	}
}

int PathLink::getMax()
{
	return Math::Max(nNormal, nReverse);
}

double PathLink::getAccumUsage(bool normalDirection, int maxUsage)
{
	if (normalDirection)
	{
		return (double)nNormal / maxUsage;
	}
	else
	{
		return (double)nReverse / maxUsage;
	}	
}

void PathLink::draw(Mat* image, Scalar color, int maxUsage, bool animate)
{
	int x1 = (int)node1->x;
	int y1 = (int)node1->y;
	int x2 = (int)node2->x;
	int y2 = (int)node2->y;
	Point point;
	double animPos2;

	if (animate)
	{
		if (nNormal >= nReverse)
		{
			animPos2 = animPos;
			animPos += (double)(nNormal - nReverse) / maxUsage;
		}
		else
		{
			animPos2 = 1 - animPos;
			animPos += (double)(nReverse - nNormal) / maxUsage;
		}

		while (animPos >= 1)
		{
			animPos -= 1;
		}

		point.x = (int)(x1 + animPos2 * (x2 - x1));
		point.y = (int)(y1 + animPos2 * (y2 - y1));

		drawMarker(*image, point, color, MARKER_CROSS, 2, 1, CV_AA);
	}
	else
	{
		line(*image, cv::Point(x1, y1), cv::Point(x2, y2), color, 1, CV_AA);
	}
}
