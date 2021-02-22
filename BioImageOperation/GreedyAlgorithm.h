#pragma once
#include "TrackingAlgorithm.h"


class GreedyAlgorithm : public TrackingAlgorithm
{
private:
	vector<vector<TrackClusterMatch*>> trackMatches;

public:
	GreedyAlgorithm();
	virtual vector<TrackClusterMatch*> solve() override;
	virtual string getDebugInfo() override;
	virtual void clear() override;

	vector<TrackClusterMatch*> calcTrackClusterMatches(Track* track, double maxMove);
	vector<TrackClusterMatch*> assignTracks();
};
