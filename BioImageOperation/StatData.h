/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once

#include <vector>
#include "Constants.h"
#include "ParamRange.h"


/*
 * Advanced statistics on gathered data - used for automated parameter definition
 */
 
class StatData
{
public:
	std::vector<double> data;

	int bins[Constants::statBins] = {};
	int nbins = 0;
	double maxRange = 0;
	double mean = 0;
	double median = 0;
	double otsu = 0;
	double peak = 0;
	double maxVal = 0;
	double stdDev = 0;
	double stdDevPos = 0;
	double stdDevNeg = 0;
	double ssstdDev = 0;

	StatData();
	void reset();
	void add(double x);
	bool calcStats();
	void calcMean();
	void removeMaxRange(double range);
	void calcMedian();
	double calcPartition(double partition);
	void calcStdDev();
	void calcSSStdDev();
	void calcHistogram();
	void calcOtsu();
	void calcPeak();
	void calcMax();
	ParamRange getParamRange();
	void saveData(System::String^ filename);
};

