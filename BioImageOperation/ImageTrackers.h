/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <vector>
#include "ImageTracker.h"

using namespace std;


/*
 * Helper for vector of ImageTracker
 */

class ImageTrackers : vector<ImageTracker*>
{
public:
	~ImageTrackers();
	void reset();
	void close();
    ImageTracker* get(string id, TrackingMethod trackingMethod = TrackingMethod::Any, double fps = 0, double pixelSize = 1, double windowSize = 1, Observer* observer = nullptr);
};
