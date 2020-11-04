/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <thread>
#include <QObject>
#include "Observer.h"
#include "Constants.h"
#include "ScriptOperations.h"
#include "ScriptOperation.h"
#include "ImageItemList.h"
#include "ImageTrackers.h"
#include "AverageBuffer.h"
#include "ImageSeries.h"
#include "AccumBuffer.h"


/*
 * Main processing of the operations recevied from the script
 */

class ScriptProcessing : public QObject
{
	Q_OBJECT

signals:
	void resetUI();
	void resetImages();
	void clearStatus();
	void showStatus(int i, int tot = 0, const char* label = "");
	void showInfo(const char* info, int displayi);
	void showImage(Mat* image, int displayi);
	void showDialog(const char* message);


public:
	Observer* observer;
	std::thread* processThread;
	//QThread* processThread;
	ScriptOperations* scriptOperations = new ScriptOperations();
	ImageItemList* imageList = new ImageItemList();
	AverageBuffer* backgroundBuffer = new AverageBuffer();
	AverageBuffer* averageBuffer = new AverageBuffer();
	ImageSeries* imageSeries = new ImageSeries();
	AccumBuffer* accumBuffer = new AccumBuffer();
	ImageTrackers* imageTrackers = new ImageTrackers();
	Mat* dummyImage = new Mat();

	string basepath;
	int sourceWidth = 0;
	int sourceHeight = 0;
	double sourceFps = 0;
	int sourceFrameNumber = 0;
	double logPower = 0;
	ArgumentValue logPalette;
	bool abort = false;
	bool debugMode = false;

	ScriptProcessing();
	~ScriptProcessing();
	void registerObserver(Observer* observer);

	/*
	 * Reset class properties when (re)starting script processing
	 */
	void reset();

	/*
	 * Start processing in separate thread
	 */
	bool startProcess(string filepath, string script);
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
	double getTime(int frame);

	/*
	 * Abort thread, attempt closing output streams to prevent data loss
	 */
	void doAbort();
};
