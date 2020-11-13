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
#include "FrameOutput.h"

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
	FrameOutput* frameOutput = NULL;
	Mat image;
	Mat* imageRef = NULL;

	ScriptOperation();
	ScriptOperation(string line);
	~ScriptOperation();
	void reset();
	void extract(string line);
	void parseArguments();
	bool hasInnerOperations();
	ScriptOperation* getNextInnerOperation();
	string getArgument(ArgumentLabel label = ArgumentLabel::None);
	int getArgument(ArgumentLabel label, int defaultArgument);
	double getArgumentNumeric(ArgumentLabel label = ArgumentLabel::None, bool oneBase = false);
	bool getArgumentBoolean(ArgumentLabel label = ArgumentLabel::None);
	static OperationInfo getOperationInfo(ScriptOperationType type);
	static ArgumentType getExpectedArgumentType(ArgumentLabel argument);
	static string getOperationList();
	static void writeOperationList(string filename);

	bool initFrameSource(FrameType frameType, int apiCode, string basepath, string templatePath, string start = "", string length = "", double fps0 = 1, int interval = 1);
	void initFrameOutput(FrameType frameType, string basepath, string templatePath, string defaultExtension = "", string start = "", string length = "", double fps = 0, string codecs = "");
	void close();
};
