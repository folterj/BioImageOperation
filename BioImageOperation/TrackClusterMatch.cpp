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
	this->rangeFactor = rangeFactor;
	areaDif = cluster->calcAreaDif(track);
	areaFactor = cluster->calcAreaFactor(track, areaDif);
	angleDif = cluster->calcAngleDif(track);
	angleFactor = cluster->calcAngleFactor(track, angleDif);
	matchFactor = rangeFactor * areaFactor;
}

bool TrackClusterMatch::isAssignable() {
	return cluster->isAssignable(track->area);
}

void TrackClusterMatch::assign() {
	cluster->assign(track);
	track->assign();
}

void TrackClusterMatch::unAssign() {
	cluster->unAssign(track);
	track->unAssign();
}
