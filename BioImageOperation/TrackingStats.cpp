/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TrackingStats.h"


TrackingStats::TrackingStats() {
}

void TrackingStats::reset() {
	trackMatching.reset();
	trackDistance.reset();
	trackLifetime.reset();
	pathMatching.reset();
	nActiveTracks = 0;
}
