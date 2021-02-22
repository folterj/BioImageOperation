#include "HungarianAlgorithm.h"
#include "Util.h"

// https://en.wikipedia.org/wiki/Assignment_problem
// https://cbom.atozmath.com/example/CBOM/Assignment.aspx?he=e&q=HM&ex=4
// (sub-optimal) alternative using discrete clones? https://stackoverflow.com/questions/48108496/hungarian-algorithm-multiple-jobs-per-worker
// https://github.com/kostasl/FIMTrack


HungarianAlgorithm::HungarianAlgorithm() {
}

HungarianAlgorithm::~HungarianAlgorithm() {
	clear();
}

vector<TrackClusterMatch*> HungarianAlgorithm::solve() {
	bool ok;
	bool changed = true;
	int iterations = 0;

	init();

	subtractMinTracks();
	ok = tryMatch(true);	// equivalent to greedy algorithm?
	if (!ok) {
		subtractMinClusters();
		ok = tryMatch(true);
		while (!ok && changed) {
			if (updateMarked1()) {
				changed = subtractMinMatrix();
				ok = tryMatch(true);
			} else {
				changed = false;
			}
			iterations++;
			//cout << printCostMatrix() << endl;
		}
	}

	if (!ok) {
		tryMatch(false);
	}

	return solutionMatches;
}

string HungarianAlgorithm::getDebugInfo() {
	return printMatrix();
}

void HungarianAlgorithm::clear() {
	if (matchMatrix) {
		delete[] matchMatrix;
	}

	if (costMatrix) {
		delete[] costMatrix;
	}

	if (validMatrix) {
		delete[] validMatrix;
	}

	if (matchedMatrix) {
		delete[] matchedMatrix;
	}

	solutionMatches.clear();
}

void HungarianAlgorithm::init() {
	Track* track;
	Cluster* cluster;
	double distance, rangeFactor;
	int ii;

	nc = clusters->size();
	nt = tracks->size();
	int n = nc * nt;

	matchMatrix = new TrackClusterMatch[n]();
	costMatrix = new double[n]();
	validMatrix = new bool[n]();
	matchedMatrix = new bool[n]();

	matchedClusters.clear();
	matchedTracks.clear();

	for (int t = 0; t < nt; t++) {
		track = (*tracks)[t];
		for (int c = 0; c < nc; c++) {
			ii = c * nt + t;
			cluster = (*clusters)[c];
			distance = cluster->calcMinDistance(track);
			rangeFactor = cluster->getRangeFactor(track, distance, maxMove);
			matchMatrix[ii].set(track, cluster, distance, rangeFactor);
			validMatrix[ii] = (rangeFactor > 0);
			costMatrix[ii] = 1 - matchMatrix[ii].matchFactor;
		}
	}
}

bool HungarianAlgorithm::subtractMinTracks() {
	bool changed = false;
	double m = 0;
	double m0;
	int mc;
	int ii;

	// get min for each track
	for (int t = 0; t < nt; t++) {
		mc = -1;
		for (int c = 0; c < nc; c++) {
			ii = c * nt + t;
			if (validMatrix[ii]) {
				m0 = costMatrix[ii];
				if (m0 < m || mc < 0) {
					m = m0;
					mc = c;
				}
			}
		}
		if (mc >= 0 && m != 0) {
			for (int c = 0; c < nc; c++) {
				ii = c * nt + t;
				if (validMatrix[ii]) {
					costMatrix[c * nt + t] -= m;
				}
			}
			matchedClusters.insert(mc);
			matchedMatrix[mc * nt + t] = true;
			changed = true;
		}
	}
	return changed;
}

bool HungarianAlgorithm::subtractMinClusters() {
	bool changed = false;
	double m = 0;
	double m0;
	int mt;
	int ii;

	// get min for each cluster
	for (int c = 0; c < nc; c++) {
		mt = -1;
		for (int t = 0; t < nt; t++) {
			ii = c * nt + t;
			if (validMatrix[ii]) {
				m0 = costMatrix[ii];
				if (m0 < m || mt < 0) {
					m = m0;
					mt = t;
				}
			}
		}
		if (mt >= 0 && m != 0) {
			for (int t = 0; t < nt; t++) {
				ii = c * nt + t;
				if (validMatrix[ii]) {
					costMatrix[ii] -= m;
				}
			}
			matchedTracks.insert(mt);
			matchedMatrix[c * nt + mt] = true;
			changed = true;
		}
	}
	return changed;
}

bool HungarianAlgorithm::subtractMinMatrix() {
	bool changed = false;
	double m = 0;
	double m0;
	int mt = -1;
	int mc = -1;
	int ii;

	// get min for each cluster
	for (int c = 0; c < nc; c++) {
		if (markedClusters.find(c) != markedClusters.end()) {
			// found: cluster in marked list
			for (int t = 0; t < nt; t++) {
				if (markedTracks.find(t) == markedTracks.end()) {
					// not found: track not in marked list
					ii = c * nt + t;
					if (validMatrix[ii]) {
						m0 = costMatrix[ii];
						if (m0 < m || mc < 0) {
							m = m0;
							mc = c;
							mt = t;
						}
					}
				}
			}
		}
	}
	if (mc >= 0 && m != 0) {
		// subtract minimum
		for (int c = 0; c < nc; c++) {
			if (markedClusters.find(c) != markedClusters.end()) {
				for (int t = 0; t < nt; t++) {
					if (markedTracks.find(t) == markedTracks.end()) {
						ii = c * nt + t;
						if (validMatrix[ii]) {
							costMatrix[ii] -= m;
						}
					}
				}
			}
		}
		// add on intersections
		for (int c = 0; c < nc; c++) {
			if (markedClusters.find(c) == markedClusters.end()) {
				for (int t = 0; t < nt; t++) {
					if (markedTracks.find(t) != markedTracks.end()) {
						ii = c * nt + t;
						if (validMatrix[ii]) {
							costMatrix[ii] += m;
						}
					}
				}
			}
		}
		matchedMatrix[mc * nt + mt] = true;
		changed = true;
	}
	return changed;
}

bool HungarianAlgorithm::updateMarked1() {
	// mark all rows in which no assigned 0
	bool changed = false;
	int ii;

	markedClusters.clear();
	markedTracks.clear();

	for (int c = 0; c < nc; c++) {
		if (matchedClusters.find(c) == matchedClusters.end()) {
			// match not found -> unmatched cluster
			if (markedClusters.insert(c).second) {
				updateMarked2(c);
				changed = true;
			}
		}
	}
	return changed;
}

void HungarianAlgorithm::updateMarked2(int markedCluster) {
	// in marked rows, if any 0 cell occurs in that row, mark that column
	for (int t = 0; t < nt; t++) {
		if (costMatrix[markedCluster * nt + t] == 0) {
			if (markedTracks.insert(t).second) {
				updateMarked3(t);
			}
		}
	}
}

void HungarianAlgorithm::updateMarked3(int markedTrack) {
	// in marked columns, if any assigned 0 exists in that columns, mark that row
	int ii;

	for (int c = 0; c < nc; c++) {
		ii = c * nt + markedTrack;
		if (matchedMatrix[ii]) {
			// match not found -> unmatched cluster
			if (markedClusters.insert(c).second) {
				updateMarked2(c);
			}
		}
	}
}

bool HungarianAlgorithm::tryMatch(bool fast) {
	// * TODO: change into 'opportunistic' matching to find single solution - this effectively also forms the check if valid solution
	TrackClusterMatch* match;
	Cluster* cluster;
	int maxMatches = 0;
	int nTracksSolved = 0;
	int nMatchClusters, ii;
	bool hasValidMatch;
	double totalArea;

	solutionMatches.clear();

	for (Cluster* cluster : *clusters) {
		cluster->unAssign();
	}
	for (Track* track : *tracks) {
		track->unAssign();
	}

	// for each track: one cluster
	for (int t = 0; t < nt; t++) {
		hasValidMatch = false;
		nMatchClusters = 0;
		for (int c = 0; c < nc; c++) {
			ii = c * nt + t;
			if (validMatrix[ii]) {
				hasValidMatch = true;
				if (costMatrix[ii] == 0) {
					// allow to find any match
					nMatchClusters++;
					match = &matchMatrix[ii];
					if (match->isAssignable()) {
						match->assign();
						solutionMatches.push_back(match);
					} else if (fast) {
						return false;
					}
				}
			}
		}
		if ((nMatchClusters > 1 || (hasValidMatch && nMatchClusters == 0)) && fast) {
			return false;
		}
	}
	return true;
}

string HungarianAlgorithm::printMatrix() {
	string s, row;
	int ii;

	for (int c = 0; c < nc; c++) {
		row = "";
		for (int t = 0; t < nt; t++) {
			ii = c * nt + t;
			if (validMatrix[ii]) {
				row += Util::format("%5.1f", costMatrix[ii]);
			} else {
				row += string(5, '-');
			}
			row += " ";
		}
		s += row + "\n";
	}
	return s;
}
