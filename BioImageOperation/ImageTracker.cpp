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
#include "GreedyAlgorithm.h"
#include "HungarianAlgorithm.h"
#include "Constants.h"
#include "Util.h"
#include "NumericPath.h"
#include "ColorScale.h"
#include "Types.h"


ImageTracker::ImageTracker(string id, TrackingMethod trackingMethod, double fps, double pixelSize, double windowSize, Observer* observer) {
	this->id = id;
	this->trackingMethod = trackingMethod;
	this->fps = fps;
	this->pixelSize = pixelSize;
	this->windowSize = windowSize;
	this->observer = observer;

	if (trackingMethod == TrackingMethod::Hungarian) {
		trackingAlgorithm = new HungarianAlgorithm();
	} else {
		trackingAlgorithm = new GreedyAlgorithm();
	}
}

ImageTracker::~ImageTracker() {
	reset();
}

void ImageTracker::deleteClusters() {
	for (int i = 0; i < clusters.size(); i++) {
		delete clusters[i];
	}
	clusters.clear();
}

void ImageTracker::deleteTracks() {
	for (int i = 0; i < tracks.size(); i++) {
		delete tracks[i];
	}
	tracks.clear();

	nextTrackLabel = 0;
}

void ImageTracker::deleteTrackMatches() {
	trackingAlgorithm->clear();
	solutionMatches.clear();
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

	nextPathLabel = 0;
}

void ImageTracker::reset() {
	deletePaths();
	deleteClusters();
	deleteTracks();

	pathAge = 0;
	pathDistance = Constants::minPathDistance;

	trackingParams.reset();
	areaStats.reset();
	distanceStats.reset();
	trackingStats.reset();

	clusterParamsFinalised = false;
	trackParamsFinalised = false;
	clusterDebugMode = false;
	trackDebugMode = false;
	pathDebugMode = false;
	countPositionSet = false;
	countPosition.x = 0;
	countPosition.y = 0;
	close();
}

string ImageTracker::createClusters(Mat* image, double minArea, double maxArea, string basePath, bool clusterDebugMode) {
	string output;
	bool clustersOk;

	this->basePath = basePath;
	this->clusterDebugMode = clusterDebugMode;

	if (minArea != 0 || maxArea != 0) {
		trackingParams.area.set(minArea, maxArea);
		clusterParamsFinalised = true;
	}

	clustersOk = findClusters(image);

	if (!clusters.empty() && !clusterParamsFinalised) {
		updateClusterParams();
	}
	if (clusterDebugMode) {
		output = getClusterDebugInfo();
	}
	return output;
}

string ImageTracker::createTracks(double maxMove, int minActive, int maxInactive, string basePath, bool trackDebugMode) {
	string output;
	bool clustersOk = !clusters.empty();

	this->basePath = basePath;
	this->trackDebugMode = trackDebugMode;

	if (maxMove != 0 || minActive != 0 || maxInactive != 0) {
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
	if (trackDebugMode) {
		output = getTrackDebugInfo();
	}
	deleteTrackMatches();
	return output;
}

string ImageTracker::createPaths(double pathDistance, bool pathDebugMode) {
	string output;
	if (pathDistance < Constants::minPathDistance) {
		pathDistance = Constants::minPathDistance;
	}
	this->pathDistance = pathDistance;
	this->pathDebugMode = pathDebugMode;

	if (trackParamsFinalised) {
		matchPaths();
		updatePaths();
	}
	if (pathDebugMode) {
		output = getPathDebugInfo();
	}
	return output;
}

bool ImageTracker::findClusters(Mat* image) {
	int totArea = image->rows * image->cols;
	int area, minArea, maxArea;
	double x, y;
	Rect box;
	int minlabel = 1;	// skip initial 'full-image label' returned by connectedComponents
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

	deleteClusters();
	clusters.reserve(n - 1);
	for (int label = minlabel; label < n; label++) {
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
			} else {
				clusterMoments = Moments();
			}
			x = clusterCentroids(label, 0);
			y = clusterCentroids(label, 1);
			clusters.push_back(new Cluster(label - minlabel, x, y, area, box, &clusterMoments, &clusterRoiImage2, pixelSize));
		}
	}
	return (!clusters.empty());
}

void ImageTracker::matchClusterTracks() {
	Track* track;
	TrackClusterMatch* match;
	int maxArea = (int)trackingParams.area.getMax();
	double maxMove = trackingParams.maxMove.getMax();
	int minActive = trackingParams.minActive;
	int label;
	string message;

	trackingAlgorithm->set(&clusters, &tracks, maxMove);
	solutionMatches = trackingAlgorithm->solve();

	// assign tracks to unassigned clusters (consider as single unmerged clusters)
	for (Cluster* cluster : clusters) {
		if (!cluster->isAssigned() && cluster->area < maxArea) {
			label = nextTrackLabel++;
			track = new Track(label, minActive, fps, pixelSize, windowSize);
			tracks.push_back(track);
			cluster->assign(track);
			track->assign();
		}
	}
	// update cluster tracks
	for (Cluster* cluster : clusters) {
		for (Track* track : cluster->assignedTracks) {
			if (trackDebugMode && trackParamsFinalised) {
				if (track->isMerged && cluster->assignedTracks.size() <= 1) {
					message = Util::format("Merged cluster split: Track %d", track->label);
					cout << message << endl;
					observer->requestPause();
				}
			}
			track->update(cluster, maxArea, maxMove, trackParamsFinalised);
		}
	}

	// stats & print failed matches
	trackingStats.trackMatchRate.reset((int)tracks.size());
	trackingStats.trackMatchFactor.reset();
	trackingStats.trackDistance.reset();
	for (TrackClusterMatch* match : solutionMatches) {
		trackingStats.trackMatchFactor.addValue(match->matchFactor);
		trackingStats.trackDistance.addValue(match->distance);
	}
	for (Track* track : tracks) {
		if (track->assigned) {
			trackingStats.trackMatchRate.addOne();
		} else {
			if (trackDebugMode && trackParamsFinalised && !track->isActive()) {
				message = "Failed match:\nTrack " + track->toString();
				match = findTrackMatch(track->label);
				if (match) {
					message += "\nPreferred:\nCluster " + match->cluster->toString();
				}
				cout << message << endl;
			}
		}
	}
}

TrackClusterMatch* ImageTracker::findTrackMatch(int tracki) {
	for (TrackClusterMatch* match : solutionMatches) {
		if (match->track->label == tracki) {
			return match;
		}
	}
	return nullptr;
}

void ImageTracker::pruneTracks() {
	double maxInactive = trackingParams.maxInactive;
	Track* track;
	int i = 0;

	trackingStats.trackLifetime.reset();

	while (i < tracks.size()) {
		track = tracks[i];
		if (!track->assigned) {
			track->updateInactive();
		}
		if (maxInactive >= 0 && track->inactiveCount > maxInactive) {
			tracks.erase(tracks.begin() + i);
			delete track;
		} else {
			if (track->isActive()) {
				trackingStats.trackLifetime.addValue(track->activeCount - 1);		// after update active count already increased
			}
			i++;
		}
	}
}

void ImageTracker::matchPaths() {
	trackingStats.pathMatching.reset();

	for (Track* track : tracks) {
		if (track->dist != 0) {
			if (matchPathElement(track)) {
				trackingStats.pathMatching.addOne();
			}
			trackingStats.pathMatching.addTotal();
		}
	}
}

bool ImageTracker::matchPathElement(Track* track) {
	PathNode* matchNode = nullptr;
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
		if (clusterDebugMode) {
			areaStats.saveData(Util::combinePath(basePath, "area_cluster_data.csv"));
		}
		trackingParams.area = areaStats.getParamRange();
		clusterParamsFinalised = true;

		// clean up potentially invalid clusters
		deletePaths();
		deleteClusters();
		deleteTracks();
	}
}

void ImageTracker::updateTrackParams() {
	double dist;
	bool set = false;

	for (Track* track : tracks) {
		dist = track->dist;
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
		if (trackDebugMode) {
			distanceStats.saveData(Util::combinePath(basePath, "move_tracking_data.csv"));
		}
		trackingParams.maxMove.set(0, distanceStats.median * 4, 0);
		trackParamsFinalised = true;

		// clean up potentially invalid tracks
		deletePaths();
		deleteTracks();
	}
}

string ImageTracker::getClusterDebugInfo() {
	string s = "";
	for (Cluster* cluster : clusters) {
		s += cluster->toString() + "\n";
	}
	return s;
}

string ImageTracker::getTrackDebugInfo() {
	return trackingAlgorithm->getDebugInfo();
}

string ImageTracker::getPathDebugInfo() {
	string s = "";
	for (PathNode* node : pathNodes) {
		s += node->toString() + "\n";
	}
	return s;
}

void ImageTracker::drawClusters(Mat* source, Mat* dest, int drawMode) {
	source->copyTo(*dest);

	for (Cluster* cluster : clusters) {
		cluster->draw(dest, drawMode);
	}
}

void ImageTracker::drawTracks(Mat* source, Mat* dest, int drawMode, int ntracks) {
	source->copyTo(*dest);

	for (Track* track : tracks) {
		if (track->isActive() || trackDebugMode) {
			track->draw(dest, drawMode, ntracks);
		}
	}
}

void ImageTracker::drawPaths(Mat* source, Mat* dest, PathDrawMode drawMode, float power, Palette palette) {
	float scale, colScale;
	int maxUsage = 0;
	Scalar color;
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
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			default: color = ColorScale::getGrayScale(colScale); break;
			}

			link->draw(dest, color, maxUsage, animate);
		}
	} else {
		for (PathNode* node : pathNodes) {
			switch (drawMode) {
			case PathDrawMode::Age: scale = (float)(1.0 / node->lastUse); break;	// *** same as 1f / (total - lastuse), with lastuse only assigned to once without need to increment continuously?
			case PathDrawMode::Usage: scale = (float)node->getAccumUsage(); break;
			case PathDrawMode::Usage2: scale = (float)node->getAccumUsage2(pathAge); break;
			default: scale = 1; break;
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
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			default: color = ColorScale::getGrayScale(colScale); break;
			}

			node->draw(dest, color);
		}
	}
}

void ImageTracker::drawTrackCount(Mat* source, Mat* dest) {
	string label;
	double x = 0;
	double y = 0;
	int n = (int)tracks.size();
	Scalar color = Scalar(0xFF, 0xFF, 0xFF);
	Point position;
	Size textSize;

	source->copyTo(*dest);

	label = Util::format("Count: %d", trackingStats.trackLifetime.n);

	if (trackParamsFinalised && !countPositionSet) {
		if (tracks.size() != 0) {
			for (Track* track : tracks) {
				x += track->x;
				y += track->y;
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
	textSize = getTextSize(label, HersheyFonts::FONT_HERSHEY_SIMPLEX, 1, 1, nullptr);
	position.x -= textSize.width / 2;
	position.y -= textSize.height / 2;
	if (position.x < 0) position.x = 0;
	if (position.y < 0) position.y = 0;

	Util::drawText(dest, label, position, HersheyFonts::FONT_HERSHEY_SIMPLEX, 1, color);
}

string ImageTracker::getInfo() {
	string info = "Tracker ID: " + id + "\n\n";

	info += Util::format("Cluster min/max area = %.1f/%.1f\n", trackingParams.area.min, trackingParams.area.max);
	info += Util::format("Tracking max move/min active/max inactive = %.1f/%d/%d\n\n", trackingParams.maxMove.getMax(), trackingParams.minActive, trackingParams.maxInactive);

	info += Util::format("Tot clusters = %d\n", clusters.size());

	if (tracks.size() != 0) {
		info += Util::format("Tot tracked clusters (active) = %d (%d)\n", tracks.size(), trackingStats.trackLifetime.n);
		info += Util::format("Tracking match rate = %.3f\n\n", trackingStats.trackMatchRate.getAverage());

		info += Util::format("Average match factor = %.3f\n", trackingStats.trackMatchFactor.getAverage());
		info += Util::format("Average distance = %.3f\n", trackingStats.trackDistance.getAverage());
		info += Util::format("Average life time = %.0f\n\n", trackingStats.trackLifetime.getAverage());
	}

	if (pathNodes.size() != 0 || pathLinks.size() != 0) {
		info += Util::format("Tot path nodes = %d\n", pathNodes.size());
		info += Util::format("Tot path links = %d\n", pathLinks.size());
		info += Util::format("Path match rate = %.3f\n", trackingStats.pathMatching.getAverage());
	}

	return info;
}

void ImageTracker::saveClusters(string filename, int frame, double time, SaveFormat saveFormat, bool outputContour) {
	OutputStream* clusterStream = nullptr;
	string csv = "";
	string sfilename;
	NumericPath filepath;
	int dcol;
	int col = 0;
	int maxi = 0;
	string maincols = Cluster::getCsvHeader(outputContour);
	int nmaincols = (int)Util::split(maincols, ",").size();
	string header = "frame,time," + maincols + "\n";

	if (saveFormat == SaveFormat::Split) {
		filepath.setOutputPath(filename);
	} else {
		clusterStream = clusterStreams.get(filename, header);
	}

	if (clusterParamsFinalised) {
		if (saveFormat == SaveFormat::ByLabel) {
			for (Cluster* cluster : clusters) {
				maxi = max(cluster->getInitialLabel(), maxi);
			}
			csv += Util::format("{0},{1},", frame, time);
			for (int i = 0; i <= maxi; i++) {
				for (Cluster* cluster : clusters) {
					if (cluster->getInitialLabel() == i) {
						dcol = i - col;
						if (dcol > 0) {
							csv += string(',', dcol * nmaincols);
							col = i;
						}
						csv += cluster->getCsv(outputContour) + ",";
						col++;
					}
				}
			}
			csv += "\n";
		} else if (saveFormat == SaveFormat::ByTime) {
			for (Cluster* cluster : clusters) {
				csv += Util::format("%d,%f,%s\n", frame, time, cluster->getCsv(outputContour).c_str());
			}
		} else if (saveFormat == SaveFormat::Split) {
			for (Cluster* cluster : clusters) {
				csv = Util::format("%d,%f,%s\n", frame, time, cluster->getCsv(outputContour).c_str());
				sfilename = filepath.createFilePath(cluster->getInitialLabel());
				clusterStream = clusterStreams.get(sfilename, header);
				clusterStream->write(csv);
			}
		}

		if (saveFormat != SaveFormat::Split) {
			clusterStream->write(csv);
		}
	}
}

void ImageTracker::saveTracks(string filename, int frame, double time, SaveFormat saveFormat, bool outputContour) {
	OutputStream* trackStream = nullptr;
	Cluster* cluster = nullptr;
	string csv = "";
	string sfilename;
	NumericPath filepath;
	int dcol;
	int col = 0;
	int maxi = 0;
	string maincols = Track::getCsvHeader(outputContour);
	int nmaincols = (int)Util::split(maincols, ",").size();
	string header = "frame,time," + maincols + "\n";

	if (saveFormat == SaveFormat::Split) {
		filepath.setOutputPath(filename);
	} else {
		trackStream = trackStreams.get(filename, header);
	}

	if (trackParamsFinalised) {
		if (saveFormat == SaveFormat::ByLabel) {
			for (Track* track : tracks) {
				if (track->isActive()) {
					maxi = max(track->label, maxi);
				}
			}
			csv += Util::format("{0},{1},", frame, time);
			for (int i = 0; i <= maxi; i++) {
				for (Track* track : tracks) {
					if (track->isActive()) {
						if (track->label == i) {
							dcol = i - col;
							if (dcol > 0) {
								csv += string(',', dcol * nmaincols);
								col = i;
							}
							if (outputContour) {
								cluster = findTrackedCluster(track);
							}
							csv += track->getCsv(outputContour, cluster) + ",";
							col++;
						}
					}
				}
			}
			csv += "\n";
		} else if (saveFormat == SaveFormat::ByTime) {
			for (Track* track : tracks) {
				if (track->isActive()) {
					if (outputContour) {
						cluster = findTrackedCluster(track);
					}
					csv += Util::format("%d,%f,%s\n", frame, time, track->getCsv(outputContour, cluster).c_str());
				}
			}
		} else if (saveFormat == SaveFormat::Split) {
			for (Track* track : tracks) {
				if (track->isActive()) {
					if (outputContour) {
						cluster = findTrackedCluster(track);
					}
					csv = Util::format("%d,%f,%s\n", frame, time, track->getCsv(outputContour, cluster).c_str());
					sfilename = filepath.createFilePath(track->label);
					trackStream = trackStreams.get(sfilename, header);
					trackStream->write(csv);
				}
			}
		}

		if (saveFormat != SaveFormat::Split) {
			trackStream->write(csv);
		}
	}
}

void ImageTracker::savePaths(string filename, int frame, double time) {
	string s = "";

	pathStream.init(filename, "frame,time,label,age,usage,last use,x,y\n");

	if (trackParamsFinalised) {
		for (PathNode* node : pathNodes) {
			s += Util::format("%d,%f,%d,%d,%d,%d,%f,%f\n", frame, time, node->label, node->age, node->accumUsage, node->lastUse, node->x, node->y);
		}
		pathStream.write(s);
	}
}

void ImageTracker::saveTrackInfo(string filename, int frame, double time) {
	string s = "";

	trackInfoStream.init(filename, "Frame,Time,Clusters,Tracks,Active tracks,Match rate,Match factor,Distance,Lifetime\n");

	if (clusterParamsFinalised) {
		s += Util::format("%d,%f,%d", frame, time, clusters.size());
		if (trackParamsFinalised) {
			s += Util::format(",%d,%d,%f,%f,%f,%f",
				tracks.size(), trackingStats.trackLifetime.n, trackingStats.trackMatchRate.getAverage(), trackingStats.trackMatchFactor.getAverage(), trackingStats.trackDistance.getAverage(), trackingStats.trackLifetime.getAverage());
		}
		s += "\n";
		trackInfoStream.write(s);
	}
}

Cluster* ImageTracker::findTrackedCluster(Track* targetTrack) {
	for (Cluster* cluster : clusters) {
		for (Track* track : cluster->assignedTracks) {
			if (track == targetTrack) {
				return cluster;
			}
		}
	}
	return nullptr;
}

void ImageTracker::close() {
	clusterStreams.close();
	trackStreams.close();
	pathStream.closeStream();
	trackInfoStream.closeStream();
}
