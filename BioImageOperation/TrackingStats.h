/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include "Averager.h"


/*
 * Main tracking stats
 */
 
class TrackingStats
{
public:
	Averager trackMatching;
	Averager trackDistance;
	Averager trackLifetime;
	Averager pathMatching;
	int nActiveTracks = 0;

	void reset();

	TrackingStats();
};
