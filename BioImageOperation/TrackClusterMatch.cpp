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


TrackClusterMatch::TrackClusterMatch() {
}

TrackClusterMatch::TrackClusterMatch(Track* track, Cluster* cluster, double distance, double rangeFactor) {
	set(track, cluster, distance, rangeFactor);
}

void TrackClusterMatch::set(Track* track, Cluster* cluster, double distance, double rangeFactor) {
	bool suspectMerged = cluster->isSuspectMerged(track);

	this->track = track;
	this->cluster = cluster;
	this->distance = distance;
	this->rangeFactor = rangeFactor;

	areaDif = cluster->calcAreaDif(track);
	areaFactor = cluster->calcAreaFactor(track, areaDif);
	if (suspectMerged) {
		areaFactor = (areaFactor + 1) / 2;
	}
	lengthDif = cluster->calcLengthDif(track);
	lengthFactor = cluster->calcLengthFactor(track, lengthDif);
	if (suspectMerged) {
		lengthFactor = (lengthFactor + 1) / 2;
	}
	angleDif = cluster->calcAngleDif(track);
    angleFactor = cluster->calcAngleFactor(angleDif);
	if (suspectMerged) {
		angleFactor = (angleFactor + 1) / 2;
	}
	activeFactor = track->activeFactor();
	matchFactor = rangeFactor * areaFactor * lengthFactor * activeFactor;
}

bool TrackClusterMatch::isAssignable() {
	return cluster->isAssignable(track);
}

void TrackClusterMatch::assign() {
	cluster->assign(track);
	track->assign(matchFactor);
	assigned = true;
}

void TrackClusterMatch::unAssign() {
	cluster->unAssign(track);
	track->unAssign();
	assigned = false;
}

string TrackClusterMatch::toString() {
	string s = Util::format("Track:%d Cluster:%d Dist:%.1f Range:%.3f DArea:%.0f FArea:%.3f DLength:%.1f FLength:%.3f DAngle:%.0f FAngle:%.3f FActive:%.3f Match:%0.3f",
							track->label, cluster->clusterLabel, distance, rangeFactor, areaDif, areaFactor, lengthDif, lengthFactor, angleDif, angleFactor, activeFactor, matchFactor);
	if (assigned) {
		s += " *";
	}
	return s;
}
