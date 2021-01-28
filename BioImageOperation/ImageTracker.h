/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <opencv2/opencv.hpp>
#include "Observer.h"
#include "Cluster.h"
#include "Track.h"
#include "TrackClusterMatch.h"
#include "PathNode.h"
#include "PathLink.h"
#include "TrackingParams.h"
#include "StatData.h"
#include "TrackingStats.h"
#include "Cluster.h"
#include "OutputStream.h"

using namespace std;
using namespace cv;


/*
 * Image tracking - clustering, tracking, common paths
 */

class ImageTracker
{
public:
	Observer* observer;
	string trackerId = "";
	string basePath = "";
	vector<vector<Point>> contours;
	vector<Cluster*> clusters;
	vector<Track*> tracks;
	vector<vector<TrackClusterMatch*>> trackMatches;
	vector<PathNode*> pathNodes;
	vector<PathLink*> pathLinks;
	int nextTrackLabel = 0;
	int nextPathLabel = 0;

	bool clusterParamsFinalised = false;
	bool trackParamsFinalised = false;
	bool countPositionSet = false;
	double pathDistance = Constants::minPathDistance;
	int pathAge = 0;
	Point countPosition;
	OutputStream clusterStream, trackStream, pathStream, trackInfoStream;
	Mat clusterLabelImage, clusterRoiImage;
	Mat1i clusterStats;
	Mat1d clusterCentroids;
	Moments clusterMoments;

	TrackingParams trackingParams;
	StatData areaStats;
	StatData distanceStats;
	TrackingStats trackingStats;

	double fps = 0;
	double pixelSize = 1;
	double windowSize = 1;

	/*
	 * Main constructor
	 */
	ImageTracker(string trackerId, double fps, double pixelSize, double windowSize, Observer* observer);

	/*
	 * Destructor
	 */
	~ImageTracker();

	/*
	 * Deep delete routines
	 */
	void deleteClusters();
	void deleteTracks();
	void deletePaths();

	/*
	 * Reset class properties
	 */
	void reset();

	/*
	 * Create clusters from image entry point
	 */
	string createClusters(Mat* image, double areaMin, double areaMax, string basePath, bool debugMode);

	/*
	 * Create tracks entry point
	 */
	string createTracks(double maxMove, int minActive, int maxInactive, string basePath, bool debugMode);

	/*
	 * Create paths entry point
	 */
	string createPaths(double pathDistance, bool debugMode);

	/*
	 * Create clusters from image
	 */
	bool findClusters(Mat* image, bool debugMode);

	/*
	 * Create tracks
	 */

	void matchClusterTracks(bool findOptimalSolution, bool debugMode);
	void unAssignAll();
	bool removeMatch(vector<TrackClusterMatch*>* trackMatches, TrackClusterMatch* removeMatch);
	vector<TrackClusterMatch*> assignTracks();
	double calcMatchScore(vector<TrackClusterMatch*>* trackMatches);
	vector<TrackClusterMatch*> calcTrackClusterMatches(Track* track, double maxMoveDistance);
	void pruneTracks();
	string getClusterDebugInfo();
	string getTrackDebugInfo();

	/*
	 * Create paths
	 */
	void matchPaths();
	bool matchPathElement(Track* track);
	void updatePaths();
	void addPathLink(PathNode* node1, PathNode* node2);

	/*
	 * Update automatic clustering parameters
	 */
	void updateClusterParams(bool debugMode);

	/*
	 * Update automatic tracking parameters
	 */
	void updateTrackParams(bool debugMode);

	/*
	 * Drawing routines
	 */
	void drawClusters(Mat* source, Mat* dest, int drawMode);
	void drawTracks(Mat* source, Mat* dest, int drawMode, int ntracks);
	void drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette);
	void drawTrackCount(Mat* source, Mat* dest);

	/*
	 * Return tracking information to show in text window
	 */
	string getInfo();

	/*
	 * Save routines
	 */
	void saveClusters(string fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void saveTracks(string fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void savePaths(string fileName, int frame, double time);
	void saveTrackInfo(string fileName, int frame, double time);
	Cluster* findTrackedCluster(Track* targetTrack);

	/*
	 * Ensure closing & flushing any open streams
	 */
	void closeStreams();
};
