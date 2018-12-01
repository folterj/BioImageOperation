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
#include "Constants.h"
#include "ScriptOperations.h"
#include "ScriptOperation.h"
#include "ImageItemList.h"
#include "ImageTrackers.h"
#include "AverageBuffer.h"
#include "ImageSeries.h"
#include "AccumBuffer.h"
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace System;
using namespace System::Threading;
using namespace cv;


/*
 * Main processing of the operations recevied from the script
 */

public ref class ScriptProcessing
{
public:
	Observer^ observer;
	Thread^ processThread;
	ScriptOperations* scriptOperations = new ScriptOperations();
	ImageItemList* imageList = new ImageItemList();
	AverageBuffer* backgroundBuffer = new AverageBuffer();
	AverageBuffer* averageBuffer = new AverageBuffer();
	ImageSeries* imageSeries = new ImageSeries();
	AccumBuffer* accumBuffer = new AccumBuffer();
	ImageTrackers* imageTrackers = new ImageTrackers();
	Mat* dummyImage = new Mat();

	System::String^ basePath;
	int sourceWidth = 0;
	int sourceHeight = 0;
	double sourceFps = 0;
	double logPower = 0;
	Palette logPalette;
	bool abort = false;

	/*
	 * Initialisation
	 */
	ScriptProcessing(Observer^ observer);

	/*
	 * Destructor
	 */
	~ScriptProcessing();

	/*
	 * Reset class properties when (re)starting script processing
	 */
	void reset();

	/*
	 * Start processing in separate thread
	 */
	void startProcess(System::String^ filePath, System::String^ script);
	void processThreadMethod();

	/*
	 * Main script loop - called recursively for { internal } loops
	 */
	void processOperations(ScriptOperations* operations, ScriptOperation* prevOperation0);

	/*
	 * Process single script operation
	 */
	bool processOperation(ScriptOperation* operation, ScriptOperation* prevOperation);

	/*
	 * Helper function to get reference image, or else current image
	 */
	Mat* getLabelOrCurrentImage(ScriptOperation* operation, Mat* currentImage, bool explicitArgument);

	/*
	 * Abort thread, attempt closing output streams to prevent data loss
	 */
	void doAbort(bool tryKill);
};
