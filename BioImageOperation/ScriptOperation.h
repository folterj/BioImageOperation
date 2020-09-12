/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "Constants.h"
#include "Argument.h"
#include "OperationInfo.h"
#include "FrameSource.h"
//#include "FrameOutput.h"

using namespace cv;


class ScriptOperations;	// forward declaration


class ScriptOperation
{
public:
	ScriptOperations* innerOperations = NULL;
	ScriptOperationType operationType = ScriptOperationType::None;
	vector<Argument*> arguments;
	string line;
	string asignee;
	int lineStart = 0;
	int lineEnd = 0;
	int interval = 1;
	int offset = 0;
	int count = 0;

	FrameSource* frameSource = NULL;
	//FrameOutput* frameOutput = NULL;
	Mat image;
	Mat* imageRef = NULL;

	ScriptOperation();
	ScriptOperation(string line);
	~ScriptOperation();
	void reset();
	void extract(string line);
	void checkArguments();
	bool hasInnerOperations();
	ScriptOperation* getNextInnerOperation();
	string getArgument(string label);
	double getArgumentNumeric(string label = "", bool oneBase = false);
	bool getArgumentBoolean(string label = "");
	string getArgument(string label, string defaultArgument);
	ClusterDrawMode getClusterDrawMode(ClusterDrawMode defaultArgument);
	static OperationInfo getOperationInfo(ScriptOperationType type);
	static string getOperationList();
	static void writeOperationList(string filename);

	bool initFrameSource(FrameType frameType, int apiCode, string basePath, string templatePath, string start = "", string length = "", double fps0 = 1, int interval = 1);
	void initFrameOutput(FrameType frameType, string basePath, string templatePath, string defaultExtension = "", string start = "", string length = "", double fps = 0, string codecs = "");
	void close();
};
