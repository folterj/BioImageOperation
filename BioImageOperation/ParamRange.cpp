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

#include "ParamRange.h"


ParamRange::ParamRange()
{
	reset();
}

void ParamRange::reset()
{
	min = 0;
	max = 0;
	mean = 0;
	n = 0;
}

void ParamRange::set(double min, double max)
{
	this->min = min;
	this->max = max;
	this->mean = (min + max) / 2;
	n = 1;
}

void ParamRange::set(double min, double max, double mean)
{
	this->min = min;
	this->max = max;
	this->mean = mean;
	n = 1;
}

void ParamRange::add(double min, double max, double mean)
{
	int newn = n + 1;
	this->min = (this->min * n + min) / newn;
	this->max = (this->max * n + max) / newn;
	this->mean = (this->mean * n + mean) / newn;
	n = newn;
}

double ParamRange::getMin()
{
	return min;
}

double ParamRange::getMax()
{
	return max;
}
