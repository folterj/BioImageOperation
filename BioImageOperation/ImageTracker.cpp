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

#include "ImageTracker.h"
#include "Constants.h"
#include "Util.h"
#include "ColorScale.h"

using namespace System;
using namespace System::IO;


/*
 * Constructor for testing
 */

ImageTracker::ImageTracker()
{
	this->trackerId = "";
}

/*
 * Main constructor
 */

ImageTracker::ImageTracker(System::String^ trackerId)
{
	this->trackerId = trackerId;
}

/*
 * Destructor
 */

ImageTracker::~ImageTracker()
{
	deletePaths();
	deleteClusters();
	deleteTracks();
}

/*
 * Deep delete routines
 */

void ImageTracker::deleteClusters()
{
	for (int i = 0; i < clusters.size(); i++)
	{
		delete clusters[i];
	}
	clusters.clear();
}

void ImageTracker::deleteTracks()
{
	for (int i = 0; i < clusterTracks.size(); i++)
	{
		delete clusterTracks[i];
	}
	clusterTracks.clear();
}

void ImageTracker::deletePaths()
{
	for (int i = 0; i < pathLinks.size(); i++)
	{
		delete pathLinks[i];
	}
	pathLinks.clear();

	for (int i = 0; i < pathNodes.size(); i++)
	{
		delete pathNodes[i];
	}
	pathNodes.clear();
}

/*
 * Reset class properties
 */

void ImageTracker::reset()
{
	deletePaths();
	deleteClusters();
	deleteTracks();

	recycledLabels.clear();
	nextTrackLabel = 0;
	nextPathLabel = 0;
	pathAge = 0;
	pathDistance = Constants::minPathDistance;

	trackingParams.reset();
	areaStats.reset();
	distanceStats.reset();
	trackingStats.reset();

	clusterParamsFinalised = false;
	trackParamsFinalised = false;
	countPositionSet = false;
	countPosition.x = 0;
	countPosition.y = 0;
	closeStreams();
}

/*
 * Create clusters from image entry point
 */

bool ImageTracker::createClusters(Mat* image, double minArea, double maxArea, System::String^ basePath)
{
	bool clustersOk;

	this->basePath = basePath;

	if (minArea != 0 || maxArea != 0)
	{
		trackingParams.area.set(minArea, maxArea);
		clusterParamsFinalised = true;
	}

	clustersOk = findClusters(image);

	if (clustersOk && !clusterParamsFinalised)
	{
		updateClusterParams();
	}
	return clustersOk;
}

/*
 * Create tracks entry point
 */

void ImageTracker::createTracks(double maxMove, int minActive, int maxInactive, System::String^ basePath)
{
	bool clustersOk = (clusters.size() > 0);

	this->basePath = basePath;

	if (maxMove != 0 || minActive != 0 || maxInactive != 0)
	{
		if (minActive <= 0)
		{
			minActive = Constants::defMinActive;
		}
		if (maxInactive <= 0)
		{
			maxInactive = Constants::defMaxInactive;
		}
		trackingParams.maxMove.set(0, maxMove);
		trackingParams.minActive = minActive;
		trackingParams.maxInactive = maxInactive;
		trackParamsFinalised = true;
	}

	matchClusterTracks();
	pruneTracks();

	if (clusterParamsFinalised && !trackParamsFinalised && clustersOk)
	{
		updateTrackParams();
	}
}

/*
 * Create paths entry point
 */

void ImageTracker::createPaths(double pathDistance)
{
	if (pathDistance < Constants::minPathDistance)
	{
		pathDistance = Constants::minPathDistance;
	}
	this->pathDistance = pathDistance;

	if (trackParamsFinalised)
	{
		matchPaths();
		updatePaths();
	}
}

/*
 * Create clusters from image
 */

bool ImageTracker::findClusters(Mat* image)
{
	int area, minArea, maxArea;
	double x, y, angle;
	Rect box;
	int n;
	bool clusterOk;

	n = connectedComponentsWithStats(*image, clusterLabelImage, clusterStats, clusterCentroids);

	if (clusterParamsFinalised)
	{
		minArea = (int)trackingParams.area.getMin();
		maxArea = (int)(trackingParams.area.getMax() * Constants::maxMergedBlobs);
	}
	else
	{
		minArea = Constants::minPixels;
		maxArea = 0;
	}

	// clean up
	deleteClusters();

	clusters.reserve(n - 1);
	for (int label = 1; label < n; label++)
	{
		// skip initial 'full-image label' returned by connectedComponents
		area = clusterStats.at<int>(label, ConnectedComponentsTypes::CC_STAT_AREA);

		if (clusterParamsFinalised)
		{
			clusterOk = (area >= minArea && area < maxArea);
		}
		else
		{
			clusterOk = (area >= minArea);
		}

		if (clusterOk)
		{
			// get RoI using stats; copy part of image
			box = Rect(clusterStats.at<int>(label, ConnectedComponentsTypes::CC_STAT_LEFT),
						clusterStats.at<int>(label, ConnectedComponentsTypes::CC_STAT_TOP),
						clusterStats.at<int>(label, ConnectedComponentsTypes::CC_STAT_WIDTH),
						clusterStats.at<int>(label, ConnectedComponentsTypes::CC_STAT_HEIGHT));

			if (clusterParamsFinalised)
			{
				clusterRoiImage = clusterLabelImage(box);

				// filter label pixels only
				clusterRoiImage2 = (clusterRoiImage == label);
				// get moments
				clusterMoments = moments(clusterRoiImage2, true);
				// get angle
				angle = Util::getMomentsAngle(&clusterMoments);
			}
			x = clusterCentroids.at<double>(label, 0);
			y = clusterCentroids.at<double>(label, 1);
			clusters.push_back(new Cluster(x, y, area, angle, box, &clusterRoiImage2));
		}
	}
	return (clusters.size() > 0);
}

/*
 * Create tracks
 */

void ImageTracker::matchClusterTracks()
{
	std::vector<DistanceCluster*> clusterMinDistances;
	DistanceCluster* distanceCluster = NULL;
	ClusterTrack* track;
	int maxArea = (int)trackingParams.area.getMax();
	double maxMove = trackingParams.maxMove.getMax();
	double distance;
	bool found;
	int label;

	trackingStats.trackMatching.reset();
	trackingStats.trackMatching.setTotal((int)clusterTracks.size());
	trackingStats.trackDistance.reset();
	trackingStats.trackDistance.setTotal((int)clusterTracks.size());

	clusterMinDistances.reserve(clusterTracks.size());
	// for each track find nearest cluster
	for (ClusterTrack* clusterTrack : clusterTracks)
	{
		distanceCluster = findNearestClusterDistance(clusterTrack, maxMove);
		if (distanceCluster)
		{
			clusterMinDistances.push_back(distanceCluster);
		}
	}

	// sort surest matches first
	std::sort(clusterMinDistances.begin(), clusterMinDistances.end(),
		[](DistanceCluster const* a, DistanceCluster const* b) { return a->distance < b->distance; });

	// to not over-assign clusters
	for (Cluster* cluster : clusters)
	{
		cluster->unAssign();
	}
	for (ClusterTrack* clusterTrack : clusterTracks)
	{
		clusterTrack->unAssign();
	}

	// assign tracks to best cluster
	for (DistanceCluster* clusterMinDistance : clusterMinDistances)
	{
		found = matchTrackCluster(clusterMinDistance, maxMove, distance);
		if (found)
		{
			trackingStats.trackMatching.addOne();
			trackingStats.trackDistance.addValue(distance);
		}
	}

	for (Cluster* cluster : clusters)
	{
		// assign tracks to unassigned clusters
		if (!cluster->isAssigned())
		{
			if (!recycledLabels.empty())
			{
				label = recycledLabels.front();
				recycledLabels.pop_front();
			}
			else
			{
				label = nextTrackLabel++;
			}
			track = new ClusterTrack(label);
			clusterTracks.push_back(track);
			cluster->assign(track);
		}
		// update
		for (ClusterTrack* clusterTrack : cluster->assignedTracks)
		{
			clusterTrack->update(cluster, maxArea, maxMove, trackParamsFinalised);
		}
	}

	// clean up
	for (int i = 0; i < clusterMinDistances.size(); i++)
	{
		delete clusterMinDistances[i];
	}
}

DistanceCluster* ImageTracker::findNearestClusterDistance(ClusterTrack* track, double maxMoveDistance)
{
	Cluster* cluster = NULL;
	double mindist = 0;
	double dist;
	bool first = true;

	for (Cluster* cluster0 : clusters)
	{
		dist = cluster0->calcDistance(track);
		if (cluster0->inRange(track, dist, maxMoveDistance))
		{
			// distance smaller than max move distance
			if (dist < mindist || first)
			{
				// smallest distance
				cluster = cluster0;
				mindist = dist;
				first = false;
			}
		}
	}
	if (cluster)
	{
		return new DistanceCluster(track, cluster, mindist);
	}
	return NULL;
}

bool ImageTracker::matchTrackCluster(DistanceCluster* distCluster, double maxMoveDistance, double& distance)
{
	Cluster* matchCluster = distCluster->cluster;
	ClusterTrack* track = distCluster->track;
	double mindist = 0;
	double dist;
	bool first = true;
	bool found = false;

	distance = 0;

	if (matchCluster->isAssignable(track->area) || !trackParamsFinalised)
	{
		// try preferred cluster
		matchCluster = distCluster->cluster;
		distance = distCluster->distance;
		found = true;
	}
	else
	{
		// find alternative best cluster
		for (Cluster* cluster : clusters)
		{
			if (cluster->isAssignable(track->area))
			{
				// only assign if track 'fits' into unallocated area of cluster
				dist = cluster->calcDistance(track);
				if (cluster->inRange(track, dist, maxMoveDistance))
				{
					// distance smaller than max move distance
					if (dist < mindist || first)
					{
						// smallest distance
						matchCluster = cluster;
						mindist = dist;
						distance = mindist;
						found = true;
						first = false;
					}
				}
			}
		}
	}

	if (found)
	{
		matchCluster->assign(track);
	}
	return found;
}

void ImageTracker::pruneTracks()
{
	double maxInactive = trackingParams.maxInactive;

	trackingStats.trackLifetime.reset();
	trackingStats.trackLifetime.setTotal((int)clusterTracks.size());
	trackingStats.nActiveTracks = 0;

	for (int i = 0; i < clusterTracks.size(); i++)
	{
		ClusterTrack* clusterTrack = clusterTracks[i];
		if (!clusterTrack->assigned)
		{
			clusterTrack->activeCount = 0;
			clusterTrack->inactiveCount++;
		}
		if (clusterTrack->inactiveCount > maxInactive)
		{
			logClusterTrack(clusterTrack);
			recycledLabels.push_back(clusterTrack->label);
			clusterTracks.erase(clusterTracks.begin() + i);
			delete clusterTrack;
		}

		else if (clusterTrack->isActive(trackingParams.minActive))
		{
			if (clusterTrack->activeCount == trackingParams.minActive) {
				// track has just become active
				logClusterTrack(clusterTrack);
			}
			trackingStats.nActiveTracks++;
			trackingStats.trackLifetime.addValue(clusterTrack->activeCount);
		}
	}
}

/*
 * Create paths
 */

void ImageTracker::matchPaths()
{
	trackingStats.pathMatching.reset();

	for (ClusterTrack* clusterTrack : clusterTracks)
	{
		if (clusterTrack->dist != 0)
		{
			if (matchPathElement(clusterTrack))
			{
				trackingStats.pathMatching.addOne();
			}
			trackingStats.pathMatching.addTotal();
		}
	}
}

bool ImageTracker::matchPathElement(ClusterTrack* track)
{
	PathNode* matchNode = NULL;
	double distance;
	double minDistance = 0;
	bool match = false;
	bool first = true;

	// shortcut for performance: first check last node
	matchNode = track->lastPathNode;
	if (matchNode)
	{
		// check distance from last node
		distance = matchNode->matchDistance(track, pathDistance / 2);
		if (distance >= 0)
		{
			match = true;
		}
	}

	if (!match)
	{
		for (PathNode* node : pathNodes)
		{
			distance = node->matchDistance(track, pathDistance);
			if (distance >= 0 && (distance < minDistance || first))
			{
				minDistance = distance;
				matchNode = node;
				match = true;
				first = false;
			}
		}
	}

	if (match)
	{
		matchNode->updateUse(pathAge);
	}
	else
	{
		matchNode = new PathNode(nextTrackLabel++, track);
		pathNodes.push_back(matchNode);
	}

	if (track->lastPathNode != matchNode)
	{
		if (track->lastPathNode)
		{
			addPathLink(track->lastPathNode, matchNode);
		}
		track->lastPathNode = matchNode;
	}

	return match;
}

void ImageTracker::updatePaths()
{
	for (PathNode* node : pathNodes)
	{
		node->age++;
		node->lastUse++;
	}
	pathAge++;
}

void ImageTracker::addPathLink(PathNode* node1, PathNode* node2)
{
	for (PathLink* link : pathLinks)
	{
		if (link->node1 == node1 && link->node2 == node2)
		{
			link->addMatch(true);
			return;
		}

		if (link->node1 == node2 && link->node2 == node1)
		{
			link->addMatch(false);
			return;
		}
	}

	// link not in list
	pathLinks.push_back(new PathLink(node1, node2));
}

/*
 * Update automatic clustering parameters 
 */

void ImageTracker::updateClusterParams()
{
	bool set = false;

	for (Cluster* cluster : clusters)
	{
		if (cluster->area >= Constants::minPixels)
		{
			areaStats.add(cluster->area);
			set = true;
		}
	}

	if (set)
	{
		trackingParams.area.n++;
	}

	if (trackingParams.area.n >= Constants::clusterTrainingCycles &&
		areaStats.dataSize() >= Constants::trainingDataPoints)
	{
		areaStats.calcStats();
		areaStats.saveData(Util::combinePath(basePath, "area_cluster_data.csv"));
		trackingParams.area = areaStats.getParamRange();
		clusterParamsFinalised = true;

		// clean up potentially invalid clusters
		deletePaths();
		deleteClusters();
		deleteTracks();
	}
}

/*
 * Update automatic tracking parameters
 */

void ImageTracker::updateTrackParams()
{
	double dist;
	bool set = false;

	for (ClusterTrack* clusterTrack : clusterTracks)
	{
		dist = clusterTrack->dist;
		if (dist != 0)
		{
			// skip non-moving objects
			distanceStats.add(dist);
			set = true;
		}
	}

	if (set)
	{
		trackingParams.maxMove.n++;
	}

	if (trackingParams.maxMove.n >= Constants::trackTrainingCycles &&
		distanceStats.dataSize() >= Constants::trainingDataPoints)
	{
		//distanceStats.removeMaxRange(0.1);	// remove top 10%
		distanceStats.calcStats();
		distanceStats.saveData(Util::combinePath(basePath, "move_tracking_data.csv"));
		trackingParams.maxMove.set(0, distanceStats.median * 4, 0);
		trackParamsFinalised = true;

		// clean up potentially invalid tracks
		deletePaths();
		deleteTracks();
	}
}

/*
 * Drawing routines
 */

void ImageTracker::drawClusters(Mat* source, Mat* dest, ClusterDrawMode drawMode)
{
	source->copyTo(*dest);

	for (Cluster* cluster : clusters)
	{
		cluster->draw(dest, drawMode);
	}
}

void ImageTracker::drawTracks(Mat* source, Mat* dest, ClusterDrawMode drawMode, double fps)
{
	source->copyTo(*dest);

	for (ClusterTrack* clusterTrack : clusterTracks)
	{
		clusterTrack->draw(dest, drawMode, fps);
	}
}

void ImageTracker::drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette)
{
	float scale, colScale;
	int maxUsage = 0;
	BGR color;
	bool animate;

	source->copyTo(*dest);

	if (power == 0)
	{
		power = 6;
	}

	if (drawMode == PathDrawMode::Links || drawMode == PathDrawMode::LinksMove)
	{
		animate = (drawMode == PathDrawMode::LinksMove);
		// sort highest last so drawn on top
		std::sort(pathLinks.begin(), pathLinks.end(),
			[](PathLink* a, PathLink* b) { return a->getMax() < b->getMax(); });

		for (PathLink* link : pathLinks)
		{
			maxUsage = Math::Max(link->getMax(), maxUsage);
		}

		for (PathLink* link : pathLinks)
		{
			scale = (float)link->getMax() / maxUsage;
			colScale = -log10(scale) / power;		// log: 1(E0) ... 1E-[power]

			if (colScale < 0)
			{
				colScale = 0;
			}
			if (colScale > 1)
			{
				colScale = 1;
			}

			switch (palette)
			{
			case Palette::Grayscale: color = ColorScale::getGrayScale(colScale); break;
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			}

			link->draw(dest, Util::bgrtoScalar(color), maxUsage, animate);
		}
	}
	else
	{
		for (PathNode* node : pathNodes)
		{
			switch (drawMode)
			{
			case PathDrawMode::Age: scale = (float)(1.0 / node->lastUse); break;	// *** same as 1f / (total - lastuse), with lastuse only assigned to once without need to increment continuously?
			case PathDrawMode::Usage: scale = (float)node->getAccumUsage(); break;
			case PathDrawMode::Usage2: scale = (float)node->getAccumUsage2(pathAge); break;
			}
			// 	colScale: 0...1
			colScale = -log10(scale) / power;		// log: 1(E0) ... 1E-[power]

			if (colScale < 0)
			{
				colScale = 0;
			}
			if (colScale > 1)
			{
				colScale = 1;
			}

			switch (palette)
			{
			case Palette::Grayscale: color = ColorScale::getGrayScale(colScale); break;
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			}

			node->draw(dest, Util::bgrtoScalar(color));
		}
	}
}

void ImageTracker::drawTrackInfo(Mat* source, Mat* dest)
{
	std::string label;
	double x = 0;
	double y = 0;
	int n = (int)clusterTracks.size();
	Scalar color = Scalar(0xFF, 0xFF, 0xFF);
	cv::Point position;
	cv::Size textSize;

	source->copyTo(*dest);

	label = Util::stdString(System::String::Format("Count: {0}", trackingStats.nActiveTracks));

	if (trackParamsFinalised && !countPositionSet)
	{
		if (clusterTracks.size() != 0)
		{
			for (ClusterTrack* clusterTrack : clusterTracks)
			{
				x += clusterTrack->x;
				y += clusterTrack->y;
			}
			if (n != 0)
			{
				x /= n;
				y /= n;
			}
			countPosition.x = (int)x;
			countPosition.y = (int)y;
			countPositionSet = true;
		}
	}
	position = countPosition;
	// use countPosition as centre point
	textSize = getTextSize(label, HersheyFonts::FONT_HERSHEY_SIMPLEX, 1, 1, NULL);
	position.x -= textSize.width / 2;
	position.y -= textSize.height / 2;
	if (position.x < 0) position.x = 0;
	if (position.y < 0) position.y = 0;

	putText(*dest, label, position, HersheyFonts::FONT_HERSHEY_SIMPLEX, 1, color, 1, LINE_AA);
}

/*
 * Return tracking information to show in text Form
 */

System::String^ ImageTracker::getInfo()
{
	System::String^ info = System::String::Format("Tracker ID: {0}\n\n", trackerId);

	info += System::String::Format("Cluster min/max area = {0:F0}/{1:F0}\n", trackingParams.area.min, trackingParams.area.max);
	info += System::String::Format("Tracking max move/min active/max inactive = {0:F1}/{1}/{2}\n\n", trackingParams.maxMove.getMax(), trackingParams.minActive, trackingParams.maxInactive);

	info += System::String::Format("Tot clusters = {0}\n", clusters.size());
	info += System::String::Format("Tot tracked clusters (active) = {0} ({1})\n", clusterTracks.size(), trackingStats.nActiveTracks);
	info += System::String::Format("Tracking match rate = {0:F2}\n\n", trackingStats.trackMatching.getAverage());

	info += System::String::Format("Average distance = {0:F3}\n", trackingStats.trackDistance.getAverage());
	info += System::String::Format("Average life time = {0:F0}\n\n", trackingStats.trackLifetime.getAverage());

	info += System::String::Format("Tot path nodes = {0}\n", pathNodes.size());
	info += System::String::Format("Tot path links = {0}\n", pathLinks.size());
	info += System::String::Format("Path match rate = {0:F2}\n", trackingStats.pathMatching.getAverage());

	return info;
}

/*
 * Save routines
 */

void ImageTracker::saveClusters(System::String^ filename, int i)
{
	System::String^ s = "";
	Cluster* cluster;
	int n = (int)clusters.size();

	clusterStream.init(filename, "Frame,Cluster,Area,Rad,Angle,Pos X,Pos Y\n");

	if (clusterParamsFinalised)
	{
		for (int c = 0; c < n; c++)
		{
			cluster = clusters[c];
			s += System::String::Format("{0},{1},{2},{3},{4},{5},{6}\n", i, c, cluster->area, cluster->rad, cluster->angle, cluster->x, cluster->y);
			//s += System::String::Format("{0}\n", cluster->area);
		}
		clusterStream.write(s);
	}
}

void ImageTracker::saveTracks(System::String^ filename, int i)
{
	System::String^ s = "";
	ClusterTrack* track;
	int n = (int)clusterTracks.size();

	trackStream.init(filename, "Frame,Track,Area,Rad,Orientation,Pos X,Pos Y\n");

	if (trackParamsFinalised)
	{
		for (int t = 0; t < n; t++)
		{
			track = clusterTracks[t];
			s += System::String::Format("{0},{1},{2},{3},{4},{5},{6}\n", i, track->label, track->area, track->rad, track->orientation, track->x, track->y);
		}
		trackStream.write(s);
	}
}

void ImageTracker::savePaths(System::String^ filename, int i)
{
	System::String^ s = "";
	PathNode* node;
	int n = (int)pathNodes.size();

	pathStream.init(filename, "Frame,Path,Age,Usage,Last use,Pos X,Pos Y\n");

	if (trackParamsFinalised)
	{
		for (int p = 0; p < n; p++)
		{
			node = pathNodes[p];
			s += System::String::Format("{0},{1},{2},{3},{4},{5},{6}\n", i, node->label, node->age, node->accumUsage, node->lastUse, node->x, node->y);
		}
		pathStream.write(s);
	}
}

void ImageTracker::saveTrackInfo(System::String^ filename, int i)
{
	System::String^ s = "";

	trackInfoStream.init(filename, "Frame,Clusters,Tracks,Active tracks,Matchrate,Distance,Lifetime\n");

	if (clusterParamsFinalised)
	{
		s += System::String::Format("{0},{1}", i, clusters.size());
		if (trackParamsFinalised)
		{
			s += System::String::Format(",{0},{1},{2},{3},{4}", clusterTracks.size(), trackingStats.nActiveTracks, trackingStats.trackMatching.getAverage(), trackingStats.trackDistance.getAverage(), trackingStats.trackLifetime.getAverage());
		}
		s += "\n";
		trackInfoStream.write(s);
	}
}

void ImageTracker::initLogClusterTrack(System::String^ filename)
{
	trackLogStream.init(filename, "Label,Area,Rad, Pos X,Pos Y,Active count,Inactive count\n");
}

void ImageTracker::logClusterTrack(ClusterTrack* clusterTrack) {
	System::String^ s = "";
	s += System::String::Format("{0},{1},{2},{3},{4},{5},{6}\n", clusterTrack->label, clusterTrack->area, clusterTrack->rad, clusterTrack->x, clusterTrack->y, clusterTrack->activeCount, clusterTrack->inactiveCount);
	trackLogStream.write(s);
}

/*
 * Ensure closing & flushing any open streams
 */

void ImageTracker::closeStreams()
{
	clusterStream.closeStream();
	trackStream.closeStream();
	pathStream.closeStream();
	trackInfoStream.closeStream();
	trackLogStream.closeStream();
}
