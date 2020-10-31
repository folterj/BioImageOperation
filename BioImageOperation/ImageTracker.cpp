/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <math.h>
#include "ImageTracker.h"
#include "Constants.h"
#include "Util.h"
#include "NumericPath.h"
#include "ColorScale.h"


 /*
  * Constructor for testing
  */

ImageTracker::ImageTracker(Observer* observer) {
	this->observer = observer;
	this->trackerId = "";
}

/*
 * Main constructor
 */

ImageTracker::ImageTracker(Observer* observer, string trackerId) {
	this->observer = observer;
	this->trackerId = trackerId;
}

/*
 * Destructor
 */

ImageTracker::~ImageTracker() {
	deletePaths();
	deleteClusters();
	deleteTracks();
}

/*
 * Deep delete routines
 */

void ImageTracker::deleteClusters() {
	for (int i = 0; i < clusters.size(); i++) {
		delete clusters[i];
	}
	clusters.clear();
}

void ImageTracker::deleteTracks() {
	for (int i = 0; i < clusterTracks.size(); i++) {
		delete clusterTracks[i];
	}
	clusterTracks.clear();
}

void ImageTracker::deletePaths() {
	for (int i = 0; i < pathLinks.size(); i++) {
		delete pathLinks[i];
	}
	pathLinks.clear();

	for (int i = 0; i < pathNodes.size(); i++) {
		delete pathNodes[i];
	}
	pathNodes.clear();
}

/*
 * Reset class properties
 */

void ImageTracker::reset() {
	deletePaths();
	deleteClusters();
	deleteTracks();

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

bool ImageTracker::createClusters(Mat* image, double minArea, double maxArea, string basePath, bool debugMode) {
	bool clustersOk;

	this->basePath = basePath;
	this->debugMode = debugMode;

	if (minArea != 0 || maxArea != 0) {
		trackingParams.area.set(minArea, maxArea);
		clusterParamsFinalised = true;
	}

	clustersOk = findClusters(image);

	if (clustersOk && !clusterParamsFinalised) {
		updateClusterParams();
	}
	return clustersOk;
}

/*
 * Create tracks entry point
 */

void ImageTracker::createTracks(double maxMove, int minActive, int maxInactive, string basePath) {
	bool clustersOk = (clusters.size() > 0);

	this->basePath = basePath;

	if (maxMove != 0 || minActive != 0 || maxInactive != 0) {
		if (minActive <= 0) {
			minActive = Constants::defMinActive;
		}
		if (maxInactive <= 0) {
			maxInactive = Constants::defMaxInactive;
		}
		trackingParams.maxMove.set(0, maxMove);
		trackingParams.minActive = minActive;
		trackingParams.maxInactive = maxInactive;
		trackParamsFinalised = true;
	}

	matchClusterTracks();
	pruneTracks();

	if (clusterParamsFinalised && !trackParamsFinalised && clustersOk) {
		updateTrackParams();
	}
}

/*
 * Create paths entry point
 */

void ImageTracker::createPaths(double pathDistance) {
	if (pathDistance < Constants::minPathDistance) {
		pathDistance = Constants::minPathDistance;
	}
	this->pathDistance = pathDistance;

	if (trackParamsFinalised) {
		matchPaths();
		updatePaths();
	}
}

/*
 * Create clusters from image
 */

bool ImageTracker::findClusters(Mat* image) {
	int totArea = image->rows * image->cols;
	int area, minArea, maxArea;
	double x, y, angle;
	Rect box;
	int n;
	bool clusterOk;

	n = connectedComponentsWithStats(*image, clusterLabelImage, clusterStats, clusterCentroids);

	if (clusterParamsFinalised) {
		minArea = (int)trackingParams.area.getMin();
		maxArea = (int)(trackingParams.area.getMax() * Constants::maxMergedBlobs);
	} else {
		minArea = Constants::minPixels;
		maxArea = 0;
	}

	// clean up
	deleteClusters();

	clusters.reserve(n - 1);
	for (int label = 1; label < n; label++) {
		// skip initial 'full-image label' returned by connectedComponents
		area = clusterStats(label, ConnectedComponentsTypes::CC_STAT_AREA);

		if ((double)area / totArea > Constants::maxBinaryPixelsFactor) {
			clusters.clear();
			break;
		}

		if (clusterParamsFinalised) {
			clusterOk = (area >= minArea && area < maxArea);
		} else {
			clusterOk = (area >= minArea);
		}

		if (clusterOk) {
			Mat clusterRoiImage2;
			// get RoI using stats; copy part of image
			box = Rect(clusterStats(label, ConnectedComponentsTypes::CC_STAT_LEFT),
				clusterStats(label, ConnectedComponentsTypes::CC_STAT_TOP),
				clusterStats(label, ConnectedComponentsTypes::CC_STAT_WIDTH),
				clusterStats(label, ConnectedComponentsTypes::CC_STAT_HEIGHT));

			if (clusterParamsFinalised) {
				clusterRoiImage = clusterLabelImage(box);
				// filter label pixels only
				clusterRoiImage2 = (clusterRoiImage == label);
				// get moments
				clusterMoments = moments(clusterRoiImage2, true);
				// get angle
				angle = Util::getMomentsAngle(&clusterMoments);
			}
			x = clusterCentroids(label, 0);
			y = clusterCentroids(label, 1);
			clusters.push_back(new Cluster(x, y, area, angle, box, &clusterMoments, &clusterRoiImage2));
		}
	}
	return (clusters.size() > 0);
}

/*
 * Create tracks
 */

void ImageTracker::matchClusterTracks() {
	vector<TrackClusterMatch*> trackClusterMatches;
	TrackClusterMatch* trackClusterMatch = NULL;
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

	trackClusterMatches.reserve(clusterTracks.size());
	// for each track find nearest cluster
	for (ClusterTrack* clusterTrack : clusterTracks) {
		trackClusterMatch = findTrackClusterMatch(clusterTrack, maxMove);
		if (trackClusterMatch) {
			trackClusterMatches.push_back(trackClusterMatch);
		}
	}

	// sort surest matches first
	sort(trackClusterMatches.begin(), trackClusterMatches.end(),
		[](TrackClusterMatch const* a, TrackClusterMatch const* b) { return a->matchFactor > b->matchFactor; });

	// to not over-assign clusters
	for (Cluster* cluster : clusters) {
		cluster->unAssign();
	}
	for (ClusterTrack* clusterTrack : clusterTracks) {
		clusterTrack->unAssign();
	}

	// assign tracks to best cluster
	for (TrackClusterMatch* clusterMinDistance : trackClusterMatches) {
		found = matchTrackCluster(clusterMinDistance, maxMove, distance);
		if (found) {
			trackingStats.trackMatching.addOne();
			trackingStats.trackDistance.addValue(distance);
		}
	}

	for (Cluster* cluster : clusters) {
		// assign tracks to unassigned clusters
		if (!cluster->isAssigned()) {
			label = nextTrackLabel++;
			track = new ClusterTrack(label);
			clusterTracks.push_back(track);
			cluster->assign(track);
		}
		// update
		for (ClusterTrack* clusterTrack : cluster->assignedTracks) {
			clusterTrack->update(cluster, maxArea, maxMove, trackParamsFinalised);
		}
	}

	// clean up
	for (int i = 0; i < trackClusterMatches.size(); i++) {
		delete trackClusterMatches[i];
	}
}

TrackClusterMatch* ImageTracker::findTrackClusterMatch(ClusterTrack* track, double maxMoveDistance) {
	TrackClusterMatch* trackClusterMatch = new TrackClusterMatch(track);
	Cluster* bestCluster = NULL;
	double matchFactor;
	double distFactor, distance;
	double areaFactor, areaDif;
	bool found = false;

	for (Cluster* cluster : clusters) {
		if (cluster->isAssignable(track->area)) {
			distance = cluster->calcDistance(track);
			if (cluster->inRange(track, distance, maxMoveDistance)) {
				distFactor = 1 - distance / maxMoveDistance;
				if (distFactor < 0) {
					distFactor = 0;
				}
				areaDif = cluster->calcAreaDif(track);
				areaFactor = 1 - areaDif / cluster->area;
				matchFactor = distFactor * areaFactor;
				if (matchFactor > trackClusterMatch->matchFactor || !found) {
					// smallest distance
					trackClusterMatch->cluster = cluster;
					trackClusterMatch->matchFactor = matchFactor;
					trackClusterMatch->distance = distance;
					trackClusterMatch->areaDif = areaDif;
					found = true;
				}
			}
		}
	}
	if (found) {
		return trackClusterMatch;
	}
	return NULL;
}

bool ImageTracker::matchTrackCluster(TrackClusterMatch* trackClusterMatch, double maxMoveDistance, double& distance) {
	TrackClusterMatch* trackClusterMatch2;
	ClusterTrack* track = trackClusterMatch->track;
	Cluster* matchCluster = trackClusterMatch->cluster;
	string message;
	bool found = false;

	distance = 0;

	if (matchCluster->isAssignable(track->area) || !trackParamsFinalised) {
		// preferred cluster
		found = true;
	} else {
		// alternative best cluster
		trackClusterMatch2 = findTrackClusterMatch(track, maxMoveDistance);
		if (trackClusterMatch2 != NULL) {
			trackClusterMatch = trackClusterMatch2;
			found = true;
		}

		if (debugMode && trackParamsFinalised && !found) {
			message = "Failed match:\nTrack " + trackClusterMatch->track->toString() + "\nPreferred:\nCluster " + trackClusterMatch->cluster->toString();
			observer->showDialog(message.c_str());
		}
	}

	if (found) {
		trackClusterMatch->assign();
		distance = trackClusterMatch->distance;
	}
	return found;
}

void ImageTracker::pruneTracks() {
	double maxInactive = trackingParams.maxInactive;

	trackingStats.trackLifetime.reset();
	trackingStats.trackLifetime.setTotal((int)clusterTracks.size());
	trackingStats.nActiveTracks = 0;

	for (int i = 0; i < clusterTracks.size(); i++) {
		ClusterTrack* clusterTrack = clusterTracks[i];
		if (!clusterTrack->assigned) {
			clusterTrack->activeCount = 0;
			clusterTrack->inactiveCount++;
		}
		if (clusterTrack->inactiveCount > maxInactive) {
			logClusterTrack(clusterTrack);
			clusterTracks.erase(clusterTracks.begin() + i);
			delete clusterTrack;
		}

		else if (clusterTrack->isActive(trackingParams.minActive)) {
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

void ImageTracker::matchPaths() {
	trackingStats.pathMatching.reset();

	for (ClusterTrack* clusterTrack : clusterTracks) {
		if (clusterTrack->dist != 0) {
			if (matchPathElement(clusterTrack)) {
				trackingStats.pathMatching.addOne();
			}
			trackingStats.pathMatching.addTotal();
		}
	}
}

bool ImageTracker::matchPathElement(ClusterTrack* track) {
	PathNode* matchNode = NULL;
	double distance;
	double minDistance = 0;
	bool match = false;
	bool first = true;

	// shortcut for performance: first check last node
	matchNode = track->lastPathNode;
	if (matchNode) {
		// check distance from last node
		distance = matchNode->matchDistance(track, pathDistance / 2);
		if (distance >= 0) {
			match = true;
		}
	}

	if (!match) {
		for (PathNode* node : pathNodes) {
			distance = node->matchDistance(track, pathDistance);
			if (distance >= 0 && (distance < minDistance || first)) {
				minDistance = distance;
				matchNode = node;
				match = true;
				first = false;
			}
		}
	}

	if (match) {
		matchNode->updateUse(pathAge);
	} else {
		matchNode = new PathNode(nextTrackLabel++, track);
		pathNodes.push_back(matchNode);
	}

	if (track->lastPathNode != matchNode) {
		if (track->lastPathNode) {
			addPathLink(track->lastPathNode, matchNode);
		}
		track->lastPathNode = matchNode;
	}

	return match;
}

void ImageTracker::updatePaths() {
	for (PathNode* node : pathNodes) {
		node->age++;
		node->lastUse++;
	}
	pathAge++;
}

void ImageTracker::addPathLink(PathNode* node1, PathNode* node2) {
	for (PathLink* link : pathLinks) {
		if (link->node1 == node1 && link->node2 == node2) {
			link->addMatch(true);
			return;
		}

		if (link->node1 == node2 && link->node2 == node1) {
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

void ImageTracker::updateClusterParams() {
	bool set = false;

	for (Cluster* cluster : clusters) {
		if (cluster->area >= Constants::minPixels) {
			areaStats.add(cluster->area);
			set = true;
		}
	}

	if (set) {
		trackingParams.area.n++;
	}

	if (trackingParams.area.n >= Constants::clusterTrainingCycles &&
		areaStats.dataSize() >= Constants::trainingDataPoints) {
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

void ImageTracker::updateTrackParams() {
	double dist;
	bool set = false;

	for (ClusterTrack* clusterTrack : clusterTracks) {
		dist = clusterTrack->dist;
		if (dist != 0) {
			// skip non-moving objects
			distanceStats.add(dist);
			set = true;
		}
	}

	if (set) {
		trackingParams.maxMove.n++;
	}

	if (trackingParams.maxMove.n >= Constants::trackTrainingCycles &&
		distanceStats.dataSize() >= Constants::trainingDataPoints) {
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

void ImageTracker::drawClusters(Mat* source, Mat* dest, int drawMode) {
	source->copyTo(*dest);

	for (Cluster* cluster : clusters) {
		cluster->draw(dest, drawMode);
	}
}

void ImageTracker::drawTracks(Mat* source, Mat* dest, int drawMode, int ntracks) {
	source->copyTo(*dest);

	for (ClusterTrack* clusterTrack : clusterTracks) {
		clusterTrack->draw(dest, drawMode, ntracks);
	}
}

void ImageTracker::drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette) {
	float scale, colScale;
	int maxUsage = 0;
	BGR color;
	bool animate;

	source->copyTo(*dest);

	if (power == 0) {
		power = 6;
	}

	if (drawMode == PathDrawMode::Links || drawMode == PathDrawMode::LinksMove) {
		animate = (drawMode == PathDrawMode::LinksMove);
		// sort highest last so drawn on top
		sort(pathLinks.begin(), pathLinks.end(),
			[](PathLink* a, PathLink* b) { return a->getMax() < b->getMax(); });

		for (PathLink* link : pathLinks) {
			maxUsage = max(link->getMax(), maxUsage);
		}

		for (PathLink* link : pathLinks) {
			scale = (float)link->getMax() / maxUsage;
			colScale = -log10(scale) / power;		// log: 1(E0) ... 1E-[power]

			if (colScale < 0) {
				colScale = 0;
			}
			if (colScale > 1) {
				colScale = 1;
			}

			switch (palette) {
			case Palette::Grayscale: color = ColorScale::getGrayScale(colScale); break;
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			}

			link->draw(dest, Util::bgrtoScalar(color), maxUsage, animate);
		}
	} else {
		for (PathNode* node : pathNodes) {
			switch (drawMode) {
			case PathDrawMode::Age: scale = (float)(1.0 / node->lastUse); break;	// *** same as 1f / (total - lastuse), with lastuse only assigned to once without need to increment continuously?
			case PathDrawMode::Usage: scale = (float)node->getAccumUsage(); break;
			case PathDrawMode::Usage2: scale = (float)node->getAccumUsage2(pathAge); break;
			}
			// 	colScale: 0...1
			colScale = -log10(scale) / power;		// log: 1(E0) ... 1E-[power]

			if (colScale < 0) {
				colScale = 0;
			}
			if (colScale > 1) {
				colScale = 1;
			}

			switch (palette) {
			case Palette::Grayscale: color = ColorScale::getGrayScale(colScale); break;
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			}

			node->draw(dest, Util::bgrtoScalar(color));
		}
	}
}

void ImageTracker::drawTrackInfo(Mat* source, Mat* dest) {
	string label;
	double x = 0;
	double y = 0;
	int n = (int)clusterTracks.size();
	Scalar color = Scalar(0xFF, 0xFF, 0xFF);
	Point position;
	Size textSize;

	source->copyTo(*dest);

	label = Util::format("Count: %d", trackingStats.nActiveTracks);

	if (trackParamsFinalised && !countPositionSet) {
		if (clusterTracks.size() != 0) {
			for (ClusterTrack* clusterTrack : clusterTracks) {
				x += clusterTrack->x;
				y += clusterTrack->y;
			}
			if (n != 0) {
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

string ImageTracker::getInfo() {
	string info = "Tracker ID: " + trackerId + "\n\n";


	info += Util::format("Cluster min/max area = %.0f/%.0f\n", trackingParams.area.min, trackingParams.area.max);
	info += Util::format("Tracking max move/min active/max inactive = %.1f/%d/%d\n\n", trackingParams.maxMove.getMax(), trackingParams.minActive, trackingParams.maxInactive);

	info += Util::format("Tot clusters = %d\n", clusters.size());
	info += Util::format("Tot tracked clusters (active) = %d (%d)\n", clusterTracks.size(), trackingStats.nActiveTracks);
	info += Util::format("Tracking match rate = %.2f\n\n", trackingStats.trackMatching.getAverage());

	info += Util::format("Average distance = %.3f\n", trackingStats.trackDistance.getAverage());
	info += Util::format("Average life time = %.0f\n\n", trackingStats.trackLifetime.getAverage());

	info += Util::format("Tot path nodes = %d\n", pathNodes.size());
	info += Util::format("Tot path links = %d\n", pathLinks.size());
	info += Util::format("Path match rate = %.2f\n", trackingStats.pathMatching.getAverage());

	return info;
}

/*
 * Save routines
 */

void ImageTracker::saveClusters(string filename, int frame, double time, SaveFormat saveFormat, bool writeContour) {
	OutputStream outStream;
	string s = "";
	string header;
	string sfilename;
	NumericPath filepath;
	int nmaincols;
	int dcol;
	int col = 0;
	int maxi = 0;

	header = "Frame,Time,Label,Area,Rad,Angle,Pos X,Pos Y";
	if (writeContour) {
		header += ",Contour";
	}
	header += "\n";
	nmaincols = Util::split(header, ",").size() - 2;

	if (saveFormat != SaveFormat::Split) {
		clusterStream.init(filename, header);
	}

	if (clusterParamsFinalised) {
		if (saveFormat == SaveFormat::ByLabel) {
			for (Cluster* cluster : clusters) {
				maxi = max(cluster->getFirstLabel(), maxi);
			}
			s += Util::format("{0},{1},", frame, time);
			for (int i = 0; i <= maxi; i++) {
				for (Cluster* cluster : clusters) {
					if (cluster->getFirstLabel() == i) {
						dcol = i - col;
						if (dcol > 0) {
							s += string(',', dcol * nmaincols);
							col = i;
						}
						s += cluster->getCsv(writeContour) + ",";
						col++;
					}
				}
			}
			s += "\n";
		} else if (saveFormat == SaveFormat::ByTime) {
			for (Cluster* cluster : clusters) {
				s += Util::format("%d,%f,%s\n", frame, time, cluster->getCsv(writeContour).c_str());
			}
		} else if (saveFormat == SaveFormat::Split) {
			for (Cluster* cluster : clusters) {
				s += Util::format("%d,%f,%s\n", frame, time, cluster->getCsv(writeContour).c_str());
				filepath.setOutputPath(filename);
				sfilename = filepath.createFilePath(cluster->getFirstLabel());
				outStream.init(sfilename, header);
				outStream.write(s);
			}
		}

		if (saveFormat != SaveFormat::Split) {
			clusterStream.write(s);
		}
	}
}

void ImageTracker::saveTracks(string filename, int frame, double time, SaveFormat saveFormat, bool writeContour) {
	OutputStream outStream;
	string s = "";
	string header;
	string sfilename;
	NumericPath filepath;
	int nmaincols;
	int dcol;
	int col = 0;
	int maxi = 0;

	header = "Frame,Time,Label,Area,Rad,Orientation,Pos X,Pos Y";
	if (writeContour) {
		header += ",Contour";
	}
	header += "\n";
	nmaincols = Util::split(header, ",").size() - 2;

	if (saveFormat != SaveFormat::Split) {
		trackStream.init(filename, header);
	}

	if (trackParamsFinalised) {
		if (saveFormat == SaveFormat::ByLabel) {
			for (ClusterTrack* track : clusterTracks) {
				maxi = max(track->label, maxi);
			}
			s += Util::format("{0},{1},", frame, time);
			for (int i = 0; i <= maxi; i++) {
				for (ClusterTrack* track : clusterTracks) {
					if (track->label == i) {
						dcol = i - col;
						if (dcol > 0) {
							s += string(',', dcol * nmaincols);
							col = i;
						}
						s += track->getCsv(findTrackedCluster(track), writeContour) + ",";
						col++;
					}
				}
			}
			s += "\n";
		} else if (saveFormat == SaveFormat::ByTime) {
			for (ClusterTrack* track : clusterTracks) {
				s += Util::format("%d,%f,%s\n", frame, time, track->getCsv(findTrackedCluster(track), writeContour).c_str());
			}
		} else if (saveFormat == SaveFormat::Split) {
			for (ClusterTrack* track : clusterTracks) {
				s += Util::format("%d,%f,%s\n", frame, time, track->getCsv(findTrackedCluster(track), writeContour).c_str());
				filepath.setOutputPath(filename);
				sfilename = filepath.createFilePath(track->label);
				outStream.init(sfilename, header);
				outStream.write(s);
			}
		}

		if (saveFormat != SaveFormat::Split) {
			trackStream.write(s);
		}
	}
}

void ImageTracker::savePaths(string filename, int frame, double time) {
	string s = "";

	pathStream.init(filename, "Frame,Time,Label,Age,Usage,Last use,Pos X,Pos Y\n");

	if (trackParamsFinalised) {
		for (PathNode* node : pathNodes) {
			s += Util::format("%d,%f,%d,%d,%d,%d,%f,%f\n", frame, time, node->label, node->age, node->accumUsage, node->lastUse, node->x, node->y);
		}
		pathStream.write(s);
	}
}

void ImageTracker::saveTrackInfo(string filename, int frame, double time) {
	string s = "";

	trackInfoStream.init(filename, "Frame,Time,Clusters,Tracks,Active tracks,Matchrate,Distance,Lifetime\n");

	if (clusterParamsFinalised) {
		s += Util::format("%d,%f,%d", frame, time, clusters.size());
		if (trackParamsFinalised) {
			s += Util::format(",%d,%d,%f,%f,%f",
				clusterTracks.size(), trackingStats.nActiveTracks, trackingStats.trackMatching.getAverage(), trackingStats.trackDistance.getAverage(), trackingStats.trackLifetime.getAverage());
		}
		s += "\n";
		trackInfoStream.write(s);
	}
}

void ImageTracker::initLogClusterTrack(string filename) {
	trackLogStream.init(filename, "Label,Area,Rad,Pos X,Pos Y,Active count,Inactive count\n");
}

void ImageTracker::logClusterTrack(ClusterTrack* clusterTrack) {
	string s = "";

	if (trackLogStream.isOpen) {
		s += Util::format("%d,%f,%f,%f,%f,%d,%d\n",
			clusterTrack->label, clusterTrack->area, clusterTrack->rad, clusterTrack->x, clusterTrack->y, clusterTrack->activeCount, clusterTrack->inactiveCount);
		trackLogStream.write(s);
	}
}

Cluster* ImageTracker::findTrackedCluster(ClusterTrack* track) {
	for (Cluster* cluster : clusters) {
		for (ClusterTrack* clusterTrack : cluster->assignedTracks) {
			if (clusterTrack == track) {
				return cluster;
			}
		}
	}
	return NULL;
}

/*
 * Ensure closing & flushing any open streams
 */

void ImageTracker::closeStreams() {
	clusterStream.closeStream();
	trackStream.closeStream();
	pathStream.closeStream();
	trackInfoStream.closeStream();
	trackLogStream.closeStream();
}
