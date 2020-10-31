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
	int dataSize();
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
	void saveData(string filename);
};
