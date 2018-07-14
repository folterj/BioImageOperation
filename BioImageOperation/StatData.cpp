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

#include "StatData.h"
#include <algorithm>

using namespace System;


StatData::StatData()
{
}

void StatData::reset()
{
	data.clear();

	for (int i = 0; i < Constants::statBins; i++)
	{
		bins[i] = 0;
	}
}

void StatData::add(double x)
{
	data.push_back(x);
}

bool StatData::calcStats()
{
	if (data.size() > 0)
	{
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

void StatData::removeMaxRange(double range)
{
	int n = (int)(data.size() * range);

	if (data.size() > 0)
	{
		std::sort(data.begin(), data.end());
		for (int i = 0; i < n; i++)
		{
			data.pop_back();
		}
	}
}

void StatData::calcMean()
{
	double tot = 0;

	if (data.size() > 0)
	{
		for (double x : data)
		{
			tot += x;
		}
		mean = tot / data.size();
	}
}

void StatData::calcMedian()
{
	median = calcPartition(0.5f);
}

double StatData::calcPartition(double partition)
{
	int index;

	if (data.size() > 0)
	{
		index = (int)(data.size() * partition);
		return data[index];
	}
	return 0;
}

void StatData::calcStdDev()
{
	double totdif = 0;
	double totdifpos = 0;
	double totdifneg = 0;
	int npos = 0;
	int nneg = 0;
	double dif;

	for(double x : data)
	{
		dif = x - mean;
		totdif += dif * dif;
		if (dif > 0)
		{
			totdifpos += dif * dif;
			npos++;
		}
		else if (dif < 0)
		{
			totdifneg += dif * dif;
			nneg++;
		}
	}
	stdDev = sqrt(totdif / data.size());
	stdDevPos = sqrt(totdifpos / npos);
	stdDevNeg = sqrt(totdifneg / nneg);
}

void StatData::calcSSStdDev()
{
	double totdif = 0;
	double dif;

	for (double x : data)
	{
		dif = x - 0;
		totdif += dif * dif;
	}
	ssstdDev = sqrt(totdif / data.size());
}

void StatData::calcHistogram()
{
	double q1, q3, iqr, binsize;
	int n = (int)data.size();
	int i;

	std::sort(data.begin(), data.end());

	//maxRange = data[n - 1];
	maxRange = calcPartition(0.95);

	if (maxRange != 0)
	{
		q1 = calcPartition(0.25);
		q3 = calcPartition(0.75);
		iqr = q3 - q1;
		binsize = 2 * iqr / Math::Pow(n, (double)1 / 3);
		nbins = (int)Math::Ceiling(maxRange / binsize);

		if (nbins > Constants::statBins)
		{
			nbins = Constants::statBins;
		}

		for (double x : data)
		{
			i = (int)(x / maxRange * nbins);
			if (i >= nbins)
			{
				i = nbins - 1;
			}
			bins[i]++;
		}
	}
	else
	{
		nbins = 0;
		maxRange = 1;
	}
}

void StatData::calcOtsu()
{
	double wF, mF, between;
	double sumB = 0;
	double wB = 0;
	double maximum = 0;
	double total = 0;
	double sum1 = 0;

	otsu = 0;

	for (int i = 0; i < nbins - 1; i++)
	{
		total += bins[i];
		sum1 += i * bins[i];
	}

	for (int i = 0; i < nbins - 1; i++)
	{
		wB += bins[i];
		wF = total - wB;

		if (wB != 0 && wF != 0)
		{
			sumB = sumB + i * bins[i];
			mF = (sum1 - sumB) / wF;
			between = wB * wF * ((sumB / wB) - mF) * ((sumB / wB) - mF);
			if (between >= maximum)
			{
				otsu = (double)i / nbins * maxRange;
				maximum = between;
			}
		}
	}
}

void StatData::calcPeak()
{
	double binval;
	int mini = (int)(otsu / maxRange * nbins);
	int medi = nbins / 2;
	int maxi = nbins - 1;
	double maxval;

	if (maxi < 1)
	{
		maxi = 1;
	}

	// starting value
	maxval = bins[medi];
	peak = (double)medi / nbins * maxRange;

	// skip last bin
	for (int i = maxi - 1; i > mini; i--)
	{
		if (i >= 0 && i < nbins)
		{
			binval = bins[i];
			if (binval > maxval)
			{
				maxval = binval;
				peak = (double)i / nbins * maxRange;
			}
		}
	}
}

void StatData::calcMax()
{
	double maxval = 0;

	if (data.size() > 0)
	{
		for (int x : bins)
		{
			if (x > maxval)
			{
				maxval = x;
			}
		}
		maxVal = maxval;
	}
}

ParamRange StatData::getParamRange()
{
	ParamRange paramRange;
	double max = peak + (peak - otsu);
	
	paramRange.set(otsu, max, peak);

	return paramRange;
}

void StatData::saveData(System::String^ filename)
{
	System::String^ s = "";

	for (double x : data)
	{
		s += System::String::Format("{0}\n", x);
	}

	System::IO::File::AppendAllText(filename, s);
}
