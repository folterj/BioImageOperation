/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once


/*
 * Simple value range - providing basic stats
 */

class ParamRange
{
public:
	double min = 0;
	double max = 0;
	double mean = 0;
	int n = 0;

	ParamRange();
	void reset();
	void set(double min, double max);
	void set(double min, double max, double mean);
	void add(double min, double max, double mean);
	double getMin();
	double getMax();
};
