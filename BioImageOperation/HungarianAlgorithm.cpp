#include "HungarianAlgorithm.h"
#include "Util.h"

// https://en.wikipedia.org/wiki/Assignment_problem
// https://cbom.atozmath.com/example/CBOM/Assignment.aspx?he=e&q=HM&ex=4
// https://en.wikipedia.org/wiki/Hungarian_algorithm
// (sub-optimal) alternative using discrete clones? https://stackoverflow.com/questions/48108496/hungarian-algorithm-multiple-jobs-per-worker
// https://github.com/kostasl/FIMTrack


HungarianAlgorithm::HungarianAlgorithm() {
}

HungarianAlgorithm::~HungarianAlgorithm() {
	clear();
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

vector<TrackClusterMatch*> HungarianAlgorithm::solve() {
	bool ok;
	bool changed = true;
	int iterations = 0;

	init();

	subtractMinTracks();
	ok = tryMatch();					// single pass equivalent to greedy algorithm?
	cout << getMarkedStats() << endl;
	if (!ok) {
		subtractMinClusters();
		ok = tryMatch();
		cout << getMarkedStats() << endl;
		while (!ok && changed) {
			if (updateMarked1()) {
				changed = subtractMinMatrix();
				ok = tryMatch();
				cout << getMarkedStats() << endl;
			} else {
				changed = false;
			}
			iterations++;
			//cout << getMatrixView() << endl;
		}
	}

	cout << endl;
	return solutionMatches;
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

bool HungarianAlgorithm::tryMatch() {
	bool resolved = true;
	TrackClusterMatch* foundMatch = nullptr;
	TrackClusterMatch* match;
	int nMatches, ii, foundii;
	bool hasValidMatch, changed;

	for (TrackClusterMatch* match : solutionMatches) {
		match->unAssign();
	}
	solutionMatches.clear();

	for (int i = 0; i < nc * nt; i++) {
		matchedMatrix[i] = false;
	}

	do {
		changed = false;
		// find tracks with single cluster
		for (int t = 0; t < nt; t++) {
			hasValidMatch = false;
			nMatches = 0;
			for (int c = 0; c < nc; c++) {
				ii = c * nt + t;
				if (validMatrix[ii]) {
					hasValidMatch = true;
					if (costMatrix[ii] == 0) {
						if (!matchedMatrix[ii]) {
							match = &matchMatrix[ii];
							if (match->isAssignable()) {
								foundMatch = match;
								foundii = ii;
								nMatches++;
							}
						}
					}
				}
			}
			if (nMatches == 1) {
				foundMatch->assign();
				solutionMatches.push_back(foundMatch);
				matchedMatrix[foundii] = true;
				changed = true;
			}
		}

		for (int c = 0; c < nc; c++) {
			nMatches = 0;
			for (int t = 0; t < nt; t++) {
				ii = c * nt + t;
				if (validMatrix[ii]) {
					if (costMatrix[ii] == 0) {
						if (!matchedMatrix[ii]) {
							match = &matchMatrix[ii];
							if (match->isAssignable()) {
								foundMatch = match;
								foundii = ii;
								nMatches++;
							}
						}
					}
				}
			}
			if (nMatches == 1) {
				foundMatch->assign();
				solutionMatches.push_back(foundMatch);
				matchedMatrix[foundii] = true;
				changed = true;
			}
		}
	} while (changed);

	resolved = (solutionMatches.size() == nt);

	cout << "Solutions found: " + to_string(solutionMatches.size()) << endl;

	return resolved;
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
		if (isClusterAvailable(c)) {
			for (int t = 0; t < nt; t++) {				
				if (isTrackAvailable(t)) {
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
			if (isClusterAvailable(c)) {
				for (int t = 0; t < nt; t++) {
					if (isTrackAvailable(t)) {
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
			if (!isClusterAvailable(c)) {
				for (int t = 0; t < nt; t++) {
					if (!isTrackAvailable(t)) {
						ii = c * nt + t;
						if (validMatrix[ii]) {
							costMatrix[ii] += m;
						}
					}
				}
			}
		}
		changed = true;
	}
	return changed;
}

bool HungarianAlgorithm::updateMarked1() {
	// mark all rows in with no assigned 0
	bool changed = false;
	bool unmatched;
	int ii;

	markedClusters.clear();
	markedTracks.clear();

	for (int c = 0; c < nc; c++) {
		unmatched = true;
		for (int t = 0; t < nt; t++) {
			ii = c * nt + t;
			if (matchedMatrix[ii]) {
				unmatched = false;
				break;
			}
		}
		if (unmatched) {
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
	int ii;

	for (int t = 0; t < nt; t++) {
		ii = markedCluster * nt + t;
		if (costMatrix[ii] == 0) {
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

bool HungarianAlgorithm::listContains(unordered_set<int> list, int i) {
	return (list.find(i) != list.end());
}

bool HungarianAlgorithm::isClusterAvailable(int c) {
	return listContains(markedClusters, c);
}

bool HungarianAlgorithm::isTrackAvailable(int t) {
	return !listContains(markedTracks, t);
}

string HungarianAlgorithm::getMatrixView() {
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

string HungarianAlgorithm::getMarkedStats() {
	string s;
	int ii;
	bool marked;
	int matchedCells = 0;
	int zeroCells = 0;
	int unmatchedCells = 0;

	s += Util::format("# tracks: %d\n", tracks->size());
	s += Util::format("# clusters: %d\n\n", clusters->size());

	for (int c = 0; c < nc; c++) {
		for (int t = 0; t < nt; t++) {
			ii = c * nt + t;
			if (validMatrix[ii]) {
				if (costMatrix[ii] == 0) {
					zeroCells++;
				}
				if (matchedMatrix[ii]) {
					matchedCells++;
				} else {
					unmatchedCells++;
				}
			}
		}
	}
	s += Util::format("matched cells: %d\n", matchedCells);
	s += Util::format("unmatched cells: %d\n", unmatchedCells);
	s += Util::format("zero cells: %d\n\n", zeroCells);

	s += Util::format("marked tracks: %d\n", markedTracks.size());
	s += Util::format("marked clusters: %d\n", markedClusters.size());
	s += Util::format("masked tracks: %d\n", markedTracks.size());
	s += Util::format("masked clusters: %d\n\n", nc - markedClusters.size());

	for (int c = 0; c < nc; c++) {
		for (int t = 0; t < nt; t++) {
			ii = c * nt + t;
			if (validMatrix[ii] && costMatrix[ii] == 0) {
				// check if covered
				marked = false;
				if (!isClusterAvailable(c)) {
					marked = true;
				}
				if (!isTrackAvailable(t)) {
					marked = true;
				}
				if (!marked) {
					s += Util::format("unmarked 0 at C:%d T:%d\n", c, t);
				}
			}
		}
	}
	s += "\n";

	return s;
}

string HungarianAlgorithm::getDebugInfo() {
	return getMatrixView();
}
