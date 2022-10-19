/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <algorithm>
#include "StatData.h"
#include "Util.h"
#include "OutputStream.h"


StatData::StatData() {
}

void StatData::reset() {
	data.clear();

	for (int i = 0; i < Constants::statBins; i++) {
		bins[i] = 0;
	}
}

void StatData::add(double x) {
	data.push_back(x);
}

int StatData::dataSize() {
	return (int)data.size();
}

bool StatData::calcStats() {
	if (data.size() > 0) {
		std::sort(data.begin(), data.end());	// for median etc
		calcMean();
		calcMedian();
		calcStdDev();
		calcSSStdDev();
		calcHistogram();
		calcMax();
		calcOtsu();
		calcPeak();
		return true;
	}
	return false;
}

void StatData::removeMaxRange(double range) {
	int n;

	if (data.size() > 0) {
		n = (int)(data.size() * range);
		std::sort(data.begin(), data.end());
		for (int i = 0; i < n; i++) {
			data.pop_back();
		}
	}
}

void StatData::calcMean() {
	double tot = 0;

	if (data.size() > 0) {
		for (double x : data) {
			tot += x;
		}
		mean = tot / data.size();
	}
}

void StatData::calcMedian() {
	median = calcPartition(0.5f);
}

double StatData::calcPartition(double partition) {
	int index;

	if (data.size() > 0) {
		index = (int)(data.size() * partition);
		return data[index];
	}
	return 0;
}

void StatData::calcStdDev() {
	double totdif = 0;
	double totdifpos = 0;
	double totdifneg = 0;
	int npos = 0;
	int nneg = 0;
	double dif;

	for (double x : data) {
		dif = x - mean;
		totdif += dif * dif;
		if (dif > 0) {
			totdifpos += dif * dif;
			npos++;
		} else if (dif < 0) {
			totdifneg += dif * dif;
			nneg++;
		}
	}
	stdDev = sqrt(totdif / data.size());
	stdDevPos = sqrt(totdifpos / npos);
	stdDevNeg = sqrt(totdifneg / nneg);
}

void StatData::calcSSStdDev() {
	double totdif = 0;
	double dif;

	for (double x : data) {
		dif = x - 0;
		totdif += dif * dif;
	}
	ssstdDev = sqrt(totdif / data.size());
}

void StatData::calcHistogram() {
    //int n = (int)data.size();
	int i;

	std::sort(data.begin(), data.end());

	//maxRange = data[n - 1];
	maxRange = calcPartition(0.95);

	if (maxRange != 0) {
		for (double x : data) {
			i = (int)(x / maxRange * Constants::statBins);
			if (i >= Constants::statBins) {
				i = Constants::statBins - 1;
			}
			bins[i]++;
		}
	} else {
		maxRange = 1;
	}
}

void StatData::calcOtsu() {
	double wF, mF, between;
	double sumB = 0;
	double wB = 0;
	double maximum = 0;
	double total = 0;
	double sum1 = 0;

	otsu = 0;

	for (int i = 0; i < Constants::statBins - 1; i++) {
		total += bins[i];
		sum1 += i * bins[i];
	}

	for (int i = 0; i < Constants::statBins - 1; i++) {
		wB += bins[i];
		wF = total - wB;

		if (wB != 0 && wF != 0) {
			sumB = sumB + i * bins[i];
			mF = (sum1 - sumB) / wF;
			between = wB * wF * ((sumB / wB) - mF) * ((sumB / wB) - mF);
			if (between >= maximum) {
				otsu = (double)i / Constants::statBins * maxRange;
				maximum = between;
			}
		}
	}
}

void StatData::calcPeak() {
	double binval;
	int mini = (int)(otsu / maxRange * Constants::statBins);
	int medi = Constants::statBins / 2;
	int maxi = Constants::statBins - 1;
	double maxval;

	if (maxi < 1) {
		maxi = 1;
	}
	if (medi < mini) {
		medi = mini;
	}

	// starting value
	maxval = bins[medi];
	peak = (double)medi / Constants::statBins * maxRange;

	// skip last bin
	for (int i = maxi - 1; i > mini; i--) {
		if (i >= 0 && i < Constants::statBins) {
			binval = bins[i];
			if (binval > maxval) {
				maxval = binval;
				peak = (double)i / Constants::statBins * maxRange;
			}
		}
	}
}

void StatData::calcMax() {
	double maxval = 0;

	if (data.size() > 0) {
		for (int x : bins) {
			if (x > maxval) {
				maxval = x;
			}
		}
		maxVal = maxval;
	}
}

ParamRange StatData::getParamRange() {
	ParamRange paramRange;
	double max = peak + (peak - otsu);

	paramRange.set(otsu, max, peak);

	return paramRange;
}

void StatData::saveData(string filename) {
	OutputStream outStream(filename);
	string s = "";

	for (double x : data) {
		s += Util::format("%f\n", x);
	}

	outStream.write(s);
}
