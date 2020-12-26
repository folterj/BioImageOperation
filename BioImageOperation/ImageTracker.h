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
#include "ClusterTrack.h"
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
	vector<ClusterTrack*> clusterTracks;
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
	OutputStream clusterStream, trackStream, pathStream, trackInfoStream, trackLogStream;
	Mat clusterLabelImage, clusterRoiImage;
	Mat1i clusterStats;
	Mat1d clusterCentroids;
	Moments clusterMoments;

	TrackingParams trackingParams;
	StatData areaStats;
	StatData distanceStats;
	TrackingStats trackingStats;

	bool debugMode = false;

	/*
	 * Constructor for testing
	 */
	ImageTracker(Observer* observer);

	/*
	 * Main constructor
	 */
	ImageTracker(Observer* observer, string trackerId);

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
	bool createClusters(Mat* image, double areaMin, double areaMax, string basePath, bool debugMode);

	/*
	 * Create tracks entry point
	 */
	void createTracks(double maxMove, int minActive, int maxInactive, string basePath);

	/*
	 * Create paths entry point
	 */
	void createPaths(double pathDistance);

	/*
	 * Create clusters from image
	 */
	bool findClusters(Mat* image);

	/*
	 * Create tracks
	 */
	void matchClusterTracks();
	vector<TrackClusterMatch*> calcTrackClusterMatches(ClusterTrack* track, double maxMoveDistance);
	vector<vector<int>> findClashMatches(vector<vector<TrackClusterMatch*>> trackMatches);
	double trackMatchScore(vector<vector<TrackClusterMatch*>> trackMatches);
	void pruneTracks();

	/*
	 * Create paths
	 */
	void matchPaths();
	bool matchPathElement(ClusterTrack* track);
	void updatePaths();
	void addPathLink(PathNode* node1, PathNode* node2);

	/*
	 * Update automatic clustering parameters
	 */
	void updateClusterParams();

	/*
	 * Update automatic tracking parameters
	 */
	void updateTrackParams();

	/*
	 * Drawing routines
	 */
	void drawClusters(Mat* source, Mat* dest, int drawMode);
	void drawTracks(Mat* source, Mat* dest, int drawMode, int ntracks);
	void drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette);
	void drawTrackInfo(Mat* source, Mat* dest);

	/*
	 * Return tracking information to show in text Form
	 */
	string getInfo();

	/*
	 * Save routines
	 */
	void saveClusters(string fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void saveTracks(string fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void savePaths(string fileName, int frame, double time);
	void saveTrackInfo(string fileName, int frame, double time);
	void initLogClusterTrack(string fileName);
	Cluster* findTrackedCluster(ClusterTrack* track);
	void logClusterTrack(ClusterTrack* clusterTrack);

	/*
	 * Ensure closing & flushing any open streams
	 */
	void closeStreams();
};
