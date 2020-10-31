/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TrackingParams.h"
#include "Constants.h"


TrackingParams::TrackingParams() {
	reset();
}

TrackingParams::TrackingParams(TrackingParams* parameters) {
	area = parameters->area;
	maxMove = parameters->maxMove;
	minActive = parameters->minActive;
	maxInactive = parameters->maxInactive;
}

void TrackingParams::reset() {
	area.reset();
	maxMove.reset();
	maxMove.max = Constants::defMaxMove;
	minActive = Constants::defMinActive;
	maxInactive = Constants::defMaxInactive;
}
