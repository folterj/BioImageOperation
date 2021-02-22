/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "Averager.h"


Averager::Averager() {
}

void Averager::reset(int total) {
	all = 0;
	setTotal(total);
}

void Averager::addValue(double value) {
	all += value;
	n++;
}

void Averager::addOne() {
	all++;
}

void Averager::addTotal() {
	n++;
}

void Averager::addPositive() {
	all++;
	n++;
}

void Averager::addNegative() {
	n++;
}

void Averager::setTotal(int total) {
	n = total;
}

double Averager::getAverage() {
	if (n != 0) {
		return all / n;
	}
	return 0;
}
