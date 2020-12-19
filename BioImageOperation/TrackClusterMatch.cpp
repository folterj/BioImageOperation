/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TrackClusterMatch.h"


TrackClusterMatch::TrackClusterMatch(ClusterTrack* track, Cluster* cluster, double distance, double rangeFactor) {
	this->track = track;
	this->cluster = cluster;
	this->distance = distance;
	areaDif = cluster->calcAreaDif(track);
	double areaFactor = 1 - pow(areaDif / cluster->area, 2);
	matchFactor = rangeFactor * areaFactor;
}

void TrackClusterMatch::assign() {
	cluster->assign(track);
}
