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
#include "Types.h"

using namespace cv;


class ScriptOperations;	// forward declaration


class ScriptOperation
{
public:
	ScriptOperationType operationType = ScriptOperationType::None;
	vector<Argument*> arguments;
	int argumentPos = 0;
	bool positionalMode = false;
	string original, line;
	string asignee;
	string extra;
	int lineStart = 0;
	int lineEnd = 0;
	int interval = 1;
	int offset = 0;
	int count = 0;

	ScriptOperations* innerOperations = nullptr;

	bool frameSourceInit = false;
	bool frameOutputInit = false;
	FrameSource* frameSource = nullptr;
	FrameOutput* frameOutput = nullptr;
	Mat image;
	Mat* imageRef = nullptr;
	Clock::time_point start;
	double timeElapseds = 0;
	int countElapsed = 0;
	double timeElapsedInits = 0;
	int countElapsedInit = 0;

	ScriptOperation();
	~ScriptOperation();
	void resetFrameSource();
	void reset();
	void initialFinish();
	void finish();
	void extract(string original, string line);
	void parseArguments();
	bool hasInnerOperations();
	ScriptOperation* getNextInnerOperation();
	string getArgument(ArgumentLabel label = ArgumentLabel::None);
	int getArgument(ArgumentLabel label, int defaultArgument);
	double getArgumentNumeric(ArgumentLabel label = ArgumentLabel::None, bool oneBase = false);
	bool getArgumentBoolean(ArgumentLabel label = ArgumentLabel::None);
	static OperationInfo getOperationInfo(ScriptOperationType type);
	static ArgumentType getExpectedArgumentType(ArgumentLabel argument);
	static string getArgumentDescription(ArgumentLabel argument);
	static string getArgumentTypeDescription(ArgumentType type);
	static string getArgumentFullDescription(ArgumentLabel argument);
	static string getOperationDescription(bool richFormat, string operation);
	static string getOperationList(bool richFormat, string operation="");
	static string getOperationListSimple();

	bool initFrameSource(FrameType frameType, string basepath, string templatePath, int apiCode, string codecs = "", string start = "", string length = "",
						 double fps0 = 1, int interval = 1, int total = 0, int width=0, int height=0);
	void initFrameOutput(FrameType frameType, string basepath, string templatePath, string defaultExtension = "", string start = "", string length = "",
						 double fps = 0, string codecs = "");
	double getDuration();
	double getDurationInit();
	void updateBenchmarking();
	void close();

private:
	void resetNextArgument();
	Argument* getNextArgument(ArgumentLabel label);
};
