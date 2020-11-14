/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ParamRange.h"


ParamRange::ParamRange() {
	reset();
}

void ParamRange::reset() {
	min = 0;
	max = 0;
	mean = 0;
	n = 0;
}

void ParamRange::set(double min, double max) {
	this->min = min;
	this->max = max;
	this->mean = (min + max) / 2;
	n = 1;
}

void ParamRange::set(double min, double max, double mean) {
	this->min = min;
	this->max = max;
	this->mean = mean;
	n = 1;
}

void ParamRange::add(double min, double max, double mean) {
	int newn = n + 1;
	this->min = (this->min * n + min) / newn;
	this->max = (this->max * n + max) / newn;
	this->mean = (this->mean * n + mean) / newn;
	n = newn;
}

double ParamRange::getMin() {
	return min;
}

double ParamRange::getMax() {
	return max;
}
