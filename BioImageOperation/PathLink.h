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

#pragma once
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "PathNode.h"


/*
 * Link between path nodes
 */

class PathLink
{
public:
	PathNode* node1;
	PathNode* node2;
	int nNormal = 0;
	int nReverse = 0;
	double animPos = 0;

	PathLink();
	PathLink(PathNode* node1, PathNode* node2);
	void addMatch(bool normalDirection);
	int getMax();
	double getAccumUsage(bool normalDirection, int maxUsage);
	void draw(Mat* image, Scalar color, int maxUsage, bool animate);
};
