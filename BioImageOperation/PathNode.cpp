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

#include "PathNode.h"
#include "ClusterTrack.h"
#include "Util.h"


PathNode::PathNode(int label, ClusterTrack* clusterTrack)
{
	this->label = label;

	x = clusterTrack->x;
	y = clusterTrack->y;
}

void PathNode::updateUse(int pathAge)
{
	usage.push_back(pathAge);
	accumUsage++;
	lastUse = 1;
}

double PathNode::getAccumUsage()
{
	return (double)accumUsage / age;
}

double PathNode::getAccumUsage2(int totalAge)
{
	double totalUse = 0;

	for (int use : usage)
	{
		totalUse += 1.0 / (totalAge - use);
	}
	return totalUse;
}

double PathNode::matchDistance(ClusterTrack* clusterTrack, double maxDistance)
{
	double distance = Util::calcDistance(x, y, clusterTrack->x, clusterTrack->y);

	if (distance < maxDistance)
	{
		return distance;
	}
	return -1;
}

void PathNode::draw(Mat* image, Scalar color)
{
	cv::Point point((int)x, (int)y);

	drawMarker(*image, point, color, MARKER_CROSS, 2, 1, LINE_AA);
}
