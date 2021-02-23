#pragma once
#include <unordered_set>
#include "TrackingAlgorithm.h"
#include "Cluster.h"
#include "Track.h"

using namespace cv;


class HungarianAlgorithm : public TrackingAlgorithm
{
private:
	vector<TrackClusterMatch*> solutionMatches;
	TrackClusterMatch* matchMatrix;
	double* costMatrix;
	bool* validMatrix;
	bool* matchedMatrix;
	unordered_set<int> markedClusters, markedTracks;
	int nc = 0;
	int nt = 0;

public:
	HungarianAlgorithm();
	~HungarianAlgorithm();

	virtual void clear() override;
	virtual vector<TrackClusterMatch*> solve() override;
	virtual string getDebugInfo() override;
 
	void init();
	bool subtractMinTracks();
	bool subtractMinClusters();
	bool subtractMinMatrix();
	bool updateMarked1();
	void updateMarked2(int markedCluster);
	void updateMarked3(int markedTrack);
	bool tryMatch();
	bool listContains(unordered_set<int> list, int i);
	bool isClusterAvailable(int c);
	bool isTrackAvailable(int t);
	string getMatrixView();
	string getMarkedStats();
};
