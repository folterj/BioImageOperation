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
 * Class for efficient average calculation
 */

class Averager
{
public:
	double all = 0;
	int n = 0;

	Averager();
	void reset();
	void addValue(double value);
	void addOne();
	void addTotal();
	void addPositive();
	void addNegative();
	void setTotal(int total);
	double getAverage();
};
