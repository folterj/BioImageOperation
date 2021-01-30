/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TrackClusterMatch.h"
#include "Util.h"


TrackClusterMatch::TrackClusterMatch(Track* track, Cluster* cluster, double distance, double rangeFactor) {
	this->track = track;
	this->cluster = cluster;
	this->distance = distance;
	this->rangeFactor = rangeFactor;
	areaDif = cluster->calcAreaDif(track);
	areaFactor = cluster->calcAreaFactor(track, areaDif);
	lengthDif = cluster->calcLengthDif(track);
	lengthFactor = cluster->calcLengthFactor(track, lengthDif);
	angleDif = cluster->calcAngleDif(track);
	angleFactor = cluster->calcAngleFactor(track, angleDif);
	matchFactor = rangeFactor * areaFactor * lengthFactor;
}

bool TrackClusterMatch::isAssignable() {
	return cluster->isAssignable(track->area);
}

void TrackClusterMatch::assign() {
	cluster->assign(track);
	track->assign();
	assigned = true;
}

void TrackClusterMatch::unAssign() {
	cluster->unAssign(track);
	track->unAssign();
	assigned = false;
}

string TrackClusterMatch::toString() {
	string s = Util::format("Track:%d Cluster:#%d Dist:%.1f Range:%.3f DArea:%.0f FArea:%.3f DLength:%.1f FLength:%.3f DAngle:%.0f FAngle:%.3f Match:%0.3f",
							track->label, cluster->clusterLabel, distance, rangeFactor, areaDif, areaFactor, lengthDif, lengthFactor, angleDif, angleFactor, matchFactor);
	if (assigned) {
		s += " *";
	}
	return s;
}
