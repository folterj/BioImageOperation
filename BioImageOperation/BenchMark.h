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
#include <vcclr.h>
#include "Observer.h"
#include "ImageTracker.h"
#include "AccumBuffer.h"

using namespace System::Diagnostics;


/*
 * Benchmark class for testing
 */

class BenchMark
{
public:
	gcroot<Observer^> observer;
	ImageTracker* tracker = new ImageTracker();
	AccumBuffer* accumBuffer = new AccumBuffer();
	gcroot<Stopwatch^> stopwatch = gcnew Stopwatch();

	bool abort = false;

	BenchMark(Observer^ observer);
	~BenchMark();
	void reset();

	void doTest();
	void doAbort();

	void benchmarkTest();
	void gpuTest();
	void accumTest();
};
