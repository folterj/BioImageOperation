#pragma once
#include "TrackClusterMatch.h"


class TrackingAlgorithm
{
protected:
	vector<Cluster*>* clusters;
	vector<Track*>* tracks;
	double maxMove = 0;

public:
	void set(vector<Cluster*>* clusters, vector<Track*>* tracks, double maxMove);

	virtual void clear() = 0;
	virtual vector<TrackClusterMatch*> solve() = 0;
	virtual string getDebugInfo() = 0;
};
