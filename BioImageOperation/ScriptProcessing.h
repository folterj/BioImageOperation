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
#ifndef _CONSOLE
#include <QObject>
#endif
#include "Observer.h"
#include "Constants.h"
#include "ScriptOperations.h"
#include "ScriptOperation.h"
#include "ImageItemList.h"
#include "ImageTrackers.h"
#include "SimpleImageBuffer.h"
#include "ImageSeries.h"
#include "AccumBuffer.h"


/*
 * Main processing of the operations recevied from the script
 */

class ScriptProcessing
#ifndef _CONSOLE
	: public QObject
#endif
{
public:
	Observer* observer = nullptr;
	std::thread* processThread = nullptr;
	ScriptOperations* scriptOperations = new ScriptOperations();
	ImageItemList* imageList = new ImageItemList();
	SimpleImageBuffer* backgroundBuffer = new SimpleImageBuffer();
	SimpleImageBuffer* simpleBuffer = new SimpleImageBuffer();
	ImageSeries* imageSeries = new ImageSeries();
	AccumBuffer* accumBuffer = new AccumBuffer();
	ImageTrackers* imageTrackers = new ImageTrackers();
	Mat* dummyImage = new Mat();

	string basepath;
	string sourceFile;
	int sourceFilei = 0;
	int nsourceFiles = 0;
	int sourceWidth = 0;
	int sourceHeight = 0;
	double sourceFps = 0;
	int sourceFrames = 0;
	int sourceFrameNumber = 0;
	double pixelSize = 1;
	double windowSize = 1;
	double logPower = 0;
	Palette logPalette = Palette::Grayscale;
	MedianMode medianMode = MedianMode::Normal;
	OperationMode operationMode = OperationMode::Idle;
	bool useGui = true;


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
	bool startProcessNoGui(string scriptFilename);
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
	Mat* getLabelOrCurrentImage(ScriptOperation* operation, Mat* currentImage);
	OperationMode getMode();
	double getTime(int frame);
	string getSourceLabel();

	/*
	 * Abort thread, attempt closing output streams to prevent data loss
	 */
	void requestPause();
	void requestAbort();
	void doReset();
	void setMode(OperationMode mode);
	void clearStatus();
	void showStatus(int i, int tot = 0, string label = "");
	void showText(string text, int displayi, string reference = "");
	void showImage(Mat* image, int displayi, string reference = "");
	void showDialog(string message, MessageLevel level = MessageLevel::Info);
	void showOperations(ScriptOperations* operations, ScriptOperation* currentOperation);
};
