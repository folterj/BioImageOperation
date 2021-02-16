/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "FrameSource.h"
#include "Util.h"


void FrameSource::calcFrameParams(string start, string length, double fps, int interval, int total, int nframes) {
	int lengthi;

	this->fps = fps;
	this->start = Util::parseFrameTime(start, fps);
	lengthi = Util::parseFrameTime(length, fps);

	if (total > 0 && nframes <= 0 && lengthi <= 0) {
		throw invalid_argument("Source length unknown, 'Total' not supported");
	}

	// set end
	if (lengthi > 0) {
		this->end = this->start + lengthi;
		if (nframes > 0 && this->end > nframes) {
			this->end = nframes;
		}
	}
	if (this->end <= 0) {
		this->end = nframes;
	}

	// ensure length set
	if (lengthi <= 0) {
		lengthi = this->end - this->start;
	}

	// set interval
	if (total > 1 && interval == 0) {
		this->interval = (lengthi - 1) / (total - 1);
	} else if (total == 1 && interval == 0) {
		this->interval = lengthi;		// ensure only a single frame
		this->start += (int)(lengthi * 0.5);
	} else {
		this->interval = interval;
		if (total > 0) {
			this->end = this->start + total;
		}
	}
	if (this->interval <= 0) {
		this->interval = 1;
	}
}
