#include "TrackingAlgorithm.h"

void TrackingAlgorithm::set(vector<Cluster*>* clusters, vector<Track*>* tracks, double maxMove)
{
	this->clusters = clusters;
	this->tracks = tracks;
	this->maxMove = maxMove;
}
