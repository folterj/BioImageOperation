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
#include <vector>
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "Constants.h"
#include "Argument.h"
#include "OperationInfo.h"
#include "FrameSource.h"
#include "FrameOutput.h"

using namespace System;
using namespace cv;


class ScriptOperations;	// forward declaration


/*
 * Holds script parameters of a single operation
 * Also holds and checks parameters for each operation and is used to dynamically construct script instructions
 */

class ScriptOperation
{
public:
	ScriptOperations* innerOperations = NULL;
	ScriptOperationType operationType = ScriptOperationType::None;
	std::vector<Argument*> arguments;
	std::string line;
	std::string asignee;
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
	ScriptOperation(System::String^ line);
	~ScriptOperation();
	void reset();
	void extract(System::String^ line);
	void checkArguments();
	bool hasInnerOperations();
	ScriptOperation* getNextInnerOperation();
	System::String^ getArgument(ArgumentLabel label = ArgumentLabel::None);
	double getArgumentNumeric(ArgumentLabel label = ArgumentLabel::None, bool oneBase = false);
	generic<class type> type getArgument(ArgumentLabel label, type defaultArgument);
	ClusterDrawMode getClusterDrawMode(ClusterDrawMode defaultArgument);
	static OperationInfo^ getOperationInfo(ScriptOperationType type);
	static System::String^ getOperationList();
	static void writeOperationList(System::String^ filename);

	bool initFrameSource(FrameType frameType, int apiCode, System::String^ basePath, System::String^ templatePath, System::String^ start = "", System::String^ length = "", double fps0 = 1, int interval = 1);
	void initFrameOutput(FrameType frameType, System::String^ basePath, System::String^ templatePath, System::String^ defaultExtension = "", System::String^ start = "", System::String^ length = "", double fps = 0, System::String^ codecs = "");
	void close();
};
