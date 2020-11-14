/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageTrackers.h"


ImageTrackers::ImageTrackers() {
}

ImageTrackers::~ImageTrackers() {
	for (int i = 0; i < size(); i++) {
		delete at(i);
	}
	clear();
}

void ImageTrackers::reset() {
	for (int i = 0; i < size(); i++) {
		delete at(i);
	}
	clear();
}

void ImageTrackers::close() {
	for (int i = 0; i < size(); i++) {
		at(i)->closeStreams();
	}
}

ImageTracker* ImageTrackers::getTracker(Observer* observer, string trackerId, bool firstCreate) {
	for (int i = 0; i < size(); i++) {
		if (at(i)->trackerId == trackerId) {
			return at(i);
		}
	}
	if (firstCreate) {
		ImageTracker* newTracker = new ImageTracker(observer, trackerId);
		push_back(newTracker);
		return newTracker;
	}
	throw invalid_argument("Tracker with ID: '" + trackerId + "' not found");
	return NULL;
}
