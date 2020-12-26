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


ImageTracker::ImageTracker(Observer* observer) {
	this->observer = observer;
	this->trackerId = "";
}

ImageTracker::ImageTracker(Observer* observer, string trackerId) {
	this->observer = observer;
	this->trackerId = trackerId;
}

ImageTracker::~ImageTracker() {
	deletePaths();
	deleteClusters();
	deleteTracks();
}

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

void ImageTracker::createTracks(double maxMove, int minActive, int maxInactive, string basePath) {
	bool clustersOk = (clusters.size() > 0);

	this->basePath = basePath;

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
}

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

bool ImageTracker::findClusters(Mat* image) {
	int totArea = image->rows * image->cols;
	int area, minArea, maxArea;
	double x, y;
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
			} else {
				clusterMoments = Moments();
			}
			x = clusterCentroids(label, 0);
			y = clusterCentroids(label, 1);
			clusters.push_back(new Cluster(x, y, area, box, &clusterMoments, &clusterRoiImage2));
		}
	}
	return (clusters.size() > 0);
}

// https://en.wikipedia.org/wiki/Assignment_problem

// Custom algorithm used for optimal matching

void ImageTracker::matchClusterTracks() {
	vector<vector<TrackClusterMatch*>> trackMatches, newTrackMatches;
	vector<TrackClusterMatch*> trackMatch;
	vector<vector<int>> clashMatches;
	unordered_map<int, int> trackMatchIndices;
	ClusterTrack* track;
	TrackClusterMatch* match;
	Cluster* cluster;
	int maxArea = (int)trackingParams.area.getMax();
	double maxMove = trackingParams.maxMove.getMax();
	double distance;
	int label, i, ii, mini;
	string message;
	double score, newScore;
	bool foundMatch;

	trackingStats.trackMatching.reset();
	trackingStats.trackMatching.setTotal((int)clusterTracks.size());
	trackingStats.trackDistance.reset();
	trackingStats.trackDistance.setTotal((int)clusterTracks.size());

	// for each track find clusters in range, sorted by preference
	trackMatches.reserve(clusterTracks.size());
	for (ClusterTrack* track : clusterTracks) {
		track->unAssign();
		trackMatch = calcTrackClusterMatches(track, maxMove);
		if (!trackMatch.empty()) {
			trackMatches.push_back(trackMatch);
		}
	}

	// sort all tracks to match surest track first
	sort(trackMatches.begin(), trackMatches.end(),
		[](vector<TrackClusterMatch*> a, vector<TrackClusterMatch*> b) { return a[0]->matchFactor > b[0]->matchFactor; });

	// build track-match lookup list
	for (ClusterTrack* track : clusterTracks) {
		i = 0;
		for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
			if (trackMatch[0]->track == track) {
				trackMatchIndices[track->label] = i;
			}
			i++;
		}
	}

	// find optimal solution
	clashMatches = findClashMatches(trackMatches);
	score = trackMatchScore(trackMatches);
	if (!clashMatches.empty()) {
		for (vector<int> clashMatch : clashMatches) {
			// find first element as swap point
			mini = trackMatchIndices.at(clashMatch[0]);
			for (int clashTrack : clashMatch) {
				ii = trackMatchIndices.at(clashTrack);
				if (ii < mini) {
					mini = ii;
				}
			}
			for (int clashTrack : clashMatch) {
				ii = trackMatchIndices.at(clashTrack);
				if (ii != mini) {
					newTrackMatches = trackMatches;
					swap(newTrackMatches[mini], newTrackMatches[ii]);
					newScore = trackMatchScore(newTrackMatches);
					if (newScore > score) {
						trackMatches = newTrackMatches;
					}
				}
			}
		}
		// assign best matches
		trackMatchScore(trackMatches);
	}

	// assign tracks to unassigned clusters
	for (Cluster* cluster : clusters) {
		if (!cluster->isAssigned()) {
			label = nextTrackLabel++;
			track = new ClusterTrack(label);
			clusterTracks.push_back(track);
			cluster->assign(track);
		}
	}
	// update cluster tracks
	for (Cluster* cluster : clusters) {
		for (ClusterTrack* track : cluster->assignedTracks) {
			track->update(cluster, maxArea, maxMove, trackParamsFinalised);
		}
	}

	// stats & print failed matches
	for (ClusterTrack* track : clusterTracks) {
		foundMatch = false;
		if (trackMatchIndices.find(track->label) != trackMatchIndices.end()) {
			ii = trackMatchIndices[track->label];
			trackMatch = trackMatches[ii];
			match = trackMatch[0];
			foundMatch = true;
		}
		if (track->assigned) {
			trackingStats.trackMatching.addOne();
			if (foundMatch) {
				trackingStats.trackDistance.addValue(match->distance);
			}
		} else {
			if (debugMode && trackParamsFinalised && track->isActive(trackingParams.minActive)) {
				message = "Failed match:\nTrack " + track->toString();
				if (foundMatch) {
					message += "\nPreferred:\nCluster " + match->cluster->toString();
				}
				cout << message << endl;
			}
		}
	}
}

vector<TrackClusterMatch*> ImageTracker::calcTrackClusterMatches(ClusterTrack* track, double maxMoveDistance) {
	vector<TrackClusterMatch*> trackMatches;
	double distance, rangeFactor;

	for (Cluster* cluster : clusters) {
		distance = cluster->calcDistance(track);
		rangeFactor = cluster->getRangeFactor(track, distance, maxMoveDistance);
		if (rangeFactor > 0) {
			trackMatches.push_back(new TrackClusterMatch(track, cluster, distance, rangeFactor));
		}
	}

	sort(trackMatches.begin(), trackMatches.end(),
		[](TrackClusterMatch* a, TrackClusterMatch* b) { return a->matchFactor > b->matchFactor; });

	return trackMatches;
}

vector<vector<int>> ImageTracker::findClashMatches(vector<vector<TrackClusterMatch*>> trackMatches) {
	vector<vector<int>> clashMatches;
	vector<int> clashMatch;
	TrackClusterMatch* match;
	bool foundClash;
	int i;

	for (Cluster* cluster : clusters) {
		cluster->unAssign();
	}

	for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
		i = 0;
		while (true) {
			if (i >= trackMatch.size()) {
				break;
			}
			match = trackMatch[i];
			if (match->cluster->isAssignable(match->track->area) || !trackParamsFinalised) {
				match->assign();
				break;
			} else {
				match->assign();
				match->cluster->assign(NULL);
			}
			i++;
		}
	}

	for (Cluster* cluster : clusters) {
		foundClash = false;
		for (ClusterTrack* track : cluster->assignedTracks) {
			if (track == NULL) {
				foundClash = true;
			}
		}
		if (foundClash) {
			for (ClusterTrack* track : cluster->assignedTracks) {
				if (track != NULL) {
					clashMatch.push_back(track->label);
				}
			}
			clashMatches.push_back(vector<int>(clashMatch));
		}
	}

	for (Cluster* cluster : clusters) {
		cluster->unAssign();
	}

	return clashMatches;
}

double ImageTracker::trackMatchScore(vector<vector<TrackClusterMatch*>> trackMatches) {
	double score = 0;
	TrackClusterMatch* match;
	int i;

	for (Cluster* cluster : clusters) {
		cluster->unAssign();
	}

	for (vector<TrackClusterMatch*> trackMatch : trackMatches) {
		i = 0;
		while (true) {
			if (i >= trackMatch.size()) {
				break;
			}
			match = trackMatch[i];
			if (match->cluster->isAssignable(match->track->area) || !trackParamsFinalised) {
				match->assign();
				score += match->matchFactor;
				break;
			}
			i++;
		}
	}
	return score;
}

void ImageTracker::pruneTracks() {
	double maxInactive = trackingParams.maxInactive;
	ClusterTrack* track;
	int i = 0;

	trackingStats.trackLifetime.reset();
	trackingStats.trackLifetime.setTotal((int)clusterTracks.size());
	trackingStats.nActiveTracks = 0;

	while (i < clusterTracks.size()) {
		track = clusterTracks[i];
		if (!track->assigned) {
			track->activeCount = 0;
			track->inactiveCount++;
		}
		if (maxInactive >= 0 && track->inactiveCount > maxInactive) {
			logClusterTrack(track);
			clusterTracks.erase(clusterTracks.begin() + i);
			delete track;
		} else {
			if (track->isActive(trackingParams.minActive)) {
				if (track->activeCount == trackingParams.minActive) {
					// track has just become active
					logClusterTrack(track);
				}
				trackingStats.nActiveTracks++;
				trackingStats.trackLifetime.addValue(track->activeCount);
			}
			i++;
		}
	}
}

void ImageTracker::matchPaths() {
	trackingStats.pathMatching.reset();

	for (ClusterTrack* track : clusterTracks) {
		if (track->dist != 0) {
			if (matchPathElement(track)) {
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
			case Palette::Heat: color = ColorScale::getHeatScale(colScale); break;
			case Palette::Rainbow: color = ColorScale::getRainbowScale(colScale); break;
			default: color = ColorScale::getGrayScale(colScale); break;
			}

			link->draw(dest, Util::bgrtoScalar(color), maxUsage, animate);
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

	Util::drawText(dest, label, position, HersheyFonts::FONT_HERSHEY_SIMPLEX, 1, color);
}

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
				s = Util::format("%d,%f,%s\n", frame, time, cluster->getCsv(writeContour).c_str());
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
				s = Util::format("%d,%f,%s\n", frame, time, track->getCsv(findTrackedCluster(track), writeContour).c_str());
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

void ImageTracker::closeStreams() {
	clusterStream.closeStream();
	trackStream.closeStream();
	pathStream.closeStream();
	trackInfoStream.closeStream();
	trackLogStream.closeStream();
}
