#include "GreedyAlgorithm.h"

// Custom algorithm used for optimal matching


GreedyAlgorithm::GreedyAlgorithm() {
}

GreedyAlgorithm::~GreedyAlgorithm() {
	clear();
}

void GreedyAlgorithm::clear() {
	for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
		for (TrackClusterMatch* match : trackMatch) {
			delete match;
		}
		trackMatch.clear();
	}
	trackMatches.clear();
}

vector<TrackClusterMatch*> GreedyAlgorithm::solve() {
	vector<TrackClusterMatch*> trackMatch;

	// for each track find clusters in range, sorted by preference
	trackMatches.clear();
	trackMatches.reserve(tracks->size());
	for (Track* track : *tracks) {
		track->unAssign();
		trackMatch = calcTrackClusterMatches(track, maxMove);
		if (!trackMatch.empty()) {
			trackMatches.push_back(trackMatch);
		}
	}

	// sort all tracks to match surest track first
	sort(trackMatches.begin(), trackMatches.end(),
		[](vector<TrackClusterMatch*> a, vector<TrackClusterMatch*> b) { return a[0]->matchFactor > b[0]->matchFactor; });

	return assignTracks();
}

string GreedyAlgorithm::getDebugInfo() {
	string s = "";
	string smatches;
	for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
		smatches = "";
		for (TrackClusterMatch* match : trackMatch) {
			if (match->track->label >= 0) {
				smatches += match->toString() + "\n";
			}
		}
		if (smatches != "") {
			s += smatches + "\n";
		}
	}
	return s;
}

vector<TrackClusterMatch*> GreedyAlgorithm::calcTrackClusterMatches(Track* track, double maxMove) {
	vector<TrackClusterMatch*> trackMatches;
	double distance, rangeFactor;

	for (Cluster* cluster : *clusters) {
		distance = cluster->calcMinDistance(track);
		rangeFactor = cluster->getRangeFactor(track, distance, maxMove);
		if (rangeFactor > 0) {
			trackMatches.push_back(new TrackClusterMatch(track, cluster, distance, rangeFactor));
		}
	}

	sort(trackMatches.begin(), trackMatches.end(),
		[](TrackClusterMatch* a, TrackClusterMatch* b) { return a->matchFactor > b->matchFactor; });

	return trackMatches;
}

vector<TrackClusterMatch*> GreedyAlgorithm::assignTracks() {
	vector<TrackClusterMatch*> assignedMatches;

	for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
		Track* track = trackMatch[0]->track;
		if (!track->assigned) {
			for (TrackClusterMatch* match : trackMatch) {
				if (match->isAssignable()) {
					match->assign();
					assignedMatches.push_back(match);
					break;
				}
				else {
					match->isAssignable();
				}
			}
		}
	}
	return assignedMatches;
}
