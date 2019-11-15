/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once
#include "Cluster.h"
#include "ClusterTrack.h"
#include "DistanceCluster.h"
#include "PathNode.h"
#include "PathLink.h"
#include "TrackingParams.h"
#include "StatData.h"
#include "TrackingStats.h"
#include "Cluster.h"
#include "OutputStream.h"
#include <vcclr.h>
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace System;
using namespace System::Windows::Forms;
using namespace cv;


/*
 * Image tracking - clustering, tracking, common paths
 */

class ImageTracker
{
public:
	gcroot<System::String^> trackerId = "";
	gcroot<System::String^> basePath = "";
	std::vector<std::vector<cv::Point>> contours;
	std::vector<Cluster*> clusters;
	std::vector<ClusterTrack*> clusterTracks;
	std::vector<PathNode*> pathNodes;
	std::vector<PathLink*> pathLinks;
	int nextTrackLabel = 0;
	int nextPathLabel = 0;

	bool clusterParamsFinalised = false;
	bool trackParamsFinalised = false;
	bool countPositionSet = false;
	double pathDistance = Constants::minPathDistance;
	int pathAge = 0;
	cv::Point countPosition;
	OutputStream clusterStream, trackStream, pathStream, trackInfoStream, trackLogStream;
	Mat clusterLabelImage, clusterStats, clusterCentroids, clusterRoiImage, clusterRoiImage2;
	Moments clusterMoments;

	TrackingParams trackingParams;
	StatData areaStats;
	StatData distanceStats;
	TrackingStats trackingStats;

	bool debugMode = false;

	/*
	 * Constructor for testing
	 */
	ImageTracker();

	/*
	 * Main constructor
	 */
	ImageTracker(System::String^ trackerId);

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
	bool createClusters(Mat* image, double areaMin, double areaMax, System::String^ basePath, bool debugMode);

	/*
	 * Create tracks entry point
	 */
	void createTracks(double maxMove, int minActive, int maxInactive, System::String^ basePath);

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
	DistanceCluster* findNearestClusterDistance(ClusterTrack* track, double maxMoveDistance);
	bool matchTrackCluster(DistanceCluster* distCluster, double maxMoveDistance, double& distance);
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
	void drawClusters(Mat* source, Mat* dest, ClusterDrawMode drawMode);
	void drawTracks(Mat* source, Mat* dest, ClusterDrawMode drawMode, int ntracks);
	void drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette);
	void drawTrackInfo(Mat* source, Mat* dest);

	/*
	 * Return tracking information to show in text Form
	 */
	System::String^ getInfo();

	/*
	 * Save routines
	 */
	void saveClusters(System::String^ fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void saveTracks(System::String^ fileName, int frame, double time, SaveFormat byLabel, bool saveContour);
	void savePaths(System::String^ fileName, int frame, double time);
	void saveTrackInfo(System::String^ fileName, int frame, double time);
	void initLogClusterTrack(System::String^ fileName);
	Cluster* findTrackedCluster(ClusterTrack* track);
	void logClusterTrack(ClusterTrack* clusterTrack);

	/*
	 * Ensure closing & flushing any open streams
	 */
	void closeStreams();
};
