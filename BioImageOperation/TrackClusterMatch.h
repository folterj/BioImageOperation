/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include "Cluster.h"


/*
 * For tracking: storing track, candidate cluster, and corresponding distance between these
 */

class TrackClusterMatch
{
public:
	ClusterTrack* track = NULL;
	Cluster* cluster = NULL;
	double distance = 0;
	double areaDif = 0;
	double angleDif = 0;
	double rangeFactor = 0;
	double areaFactor = 0;
	double angleFactor = 0;
	double matchFactor = 0;

	TrackClusterMatch(ClusterTrack* track, Cluster* cluster, double distance, double rangeFactor);
	bool isAssignable();
	void assign();
	void unAssign();
};
