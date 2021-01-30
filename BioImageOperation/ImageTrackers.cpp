/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageTrackers.h"


ImageTrackers::~ImageTrackers() {
	close();
	reset();
}

void ImageTrackers::reset() {
	for (int i = 0; i < size(); i++) {
		delete at(i);
	}
	clear();
}

void ImageTrackers::close() {
	for (ImageTracker* tracker : *this) {
		tracker->close();
	}
}

ImageTracker* ImageTrackers::get(string id, double fps, double pixelSize, double windowSize, Observer* observer) {
	for (ImageTracker* tracker : *this) {
		if (tracker->id == id) {
			return tracker;
		}
	}
	if (observer) {
		ImageTracker* newTracker = new ImageTracker(id, fps, pixelSize, windowSize, observer);
		push_back(newTracker);
		return newTracker;
	}
	throw invalid_argument("Tracker with ID: '" + id + "' not found");
	return nullptr;
}
