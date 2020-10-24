/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <fstream>
#include <iostream>
#include "ScriptOperation.h"
#include "ScriptOperations.h"
#include "ImageSource.h"
#include "VideoSource.h"
#include "CaptureSource.h"
#include "ImageOutput.h"
#include "VideoOutput.h"
#include "Util.h"


ScriptOperation::ScriptOperation() {
}

ScriptOperation::ScriptOperation(string line) {
	extract(line);
}

ScriptOperation::~ScriptOperation() {
	if (innerOperations) {
		delete innerOperations;
	}

	for (int i = 0; i < arguments.size(); i++) {
		delete arguments.at(i);
	}

	if (frameSource) {
		delete frameSource;
	}

	if (frameOutput) {
		delete frameOutput;
	}
}

void ScriptOperation::reset() {
	if (innerOperations) {
		innerOperations->reset();
	}
}

void ScriptOperation::extract(string line) {
	string operation, part;
	int i, i1, i2;

	this->line = line;

	i = line.find(":");
	i1 = line.find("-");
	i2 = line.find("(");
	if (i > 0 && (i < i2 || i2 < 0)) {
		if (i1 > 0 && i1 < i) {
			// interval & offset
			part = line.substr(0, i1);
			if (Util::isNumeric(part)) {
				interval = stoi(part);
			}
			i1++;
			part = line.substr(i1, i - i1);
			if (Util::isNumeric(part)) {
				offset = stoi(part);
			}
		} else {
			part = line.substr(0, i);
			if (Util::isNumeric(part)) {
				// interval
				interval = stoi(part);
			}
		}
		line = Util::trim_copy(line.substr(i + 1));
	}

	i = line.find("=");
	i1 = line.find("(");
	if (i > 0 && i < i1) {
		asignee = Util::trim_copy(line.substr(0, i));
		line = Util::trim_copy(line.substr(i + 1));
	}

	i1 = line.find("(");	// get index again in shortened line
	if (i1 > 0) {
		operation = Util::trim_copy(line.substr(0, i1));
		i2 = line.find_last_of(")");
		if (i2 < 0) {
			i2 = line.size();
		}
		line = Util::trim_copy(line.substr(i1 + 1, i2 - i1 - 1));

		for (string arg : Util::split(line, ",")) {
			if (Util::trim_copy(arg) != "") {
				arguments.push_back(new Argument(Util::trim_copy(arg)));
			}
		}
	}

	if (Util::contains(ScriptOperationTypes, operation)) {
		operationType = (ScriptOperationType)Util::getListIndex(ScriptOperationTypes, operation);
		checkArguments();
	} else {
		throw invalid_argument("Unkown operation: " + operation);
	}
}

void ScriptOperation::checkArguments() {
	OperationInfo info = getOperationInfo(operationType);
	vector<ArgumentLabel> requiredArguments = info.requiredArguments;
	vector<ArgumentLabel> optionalArguments = info.optionalArguments;
	bool found;

	if (!requiredArguments.empty() && !optionalArguments.empty()) {
		// check required arguments
		for (ArgumentLabel label : requiredArguments) {
			found = false;
			for (Argument* argument : arguments) {
				if (argument->argumentLabel != ArgumentLabel::None) {
					if (argument->argumentLabel == label) {
						found = true;
					}
				} else if (requiredArguments.size() == 1 && arguments.size() == 1) {
					if (label != ArgumentLabel::Path) {
						// exception: if only single required argument and no label used (except for path as this should be found)
						found = true;
					}
				}
			} if (!found) {
				throw invalid_argument("Missing required argument: " + ArgumentLabels[(int)label]);
			}
		}

		// check all arguments
		for (Argument* argument : arguments) {
			// ignore non-labelled arguments
			if (argument->argumentLabel != ArgumentLabel::None) {
				found = false;
				for (ArgumentLabel label : requiredArguments) {
					if (label == argument->argumentLabel) {
						found = true;
					}
				} for (ArgumentLabel label : optionalArguments) {
					if (label == argument->argumentLabel) {
						found = true;
					}
				}
				if (!found) {
					throw invalid_argument("Unexpected argument: " + argument->allArgument);
				}
			}
		}
	}
}

bool ScriptOperation::hasInnerOperations() {
	if (innerOperations != NULL) {
		return innerOperations->hasOperations();
	}
	return false;
}

ScriptOperation* ScriptOperation::getNextInnerOperation() {
	ScriptOperation* innerOperation = NULL;
	if (hasInnerOperations()) {
		innerOperation = innerOperations->getCurrentOperation();
		innerOperations->moveNextOperation();
	}
	return innerOperation;
}

string ScriptOperation::getArgument(ArgumentLabel label) {
	string arg = "";

	if (label != ArgumentLabel::None) {
		// search for label
		for (Argument* argument : arguments) {
			if (argument->argumentLabel == label) {
				arg = argument->value;
				break;
			}
		}
	} else if (arguments.size() > 0) {
		// else return first argument
		arg = arguments.at(0)->value;
	}

	return arg;
}

string ScriptOperation::getArgument(ArgumentLabel label, string defaultArgument) {
	string arg = getArgument(label);
	if (arg != "") {
		arg = defaultArgument;
	}
	return arg;
}

double ScriptOperation::getArgumentNumeric(ArgumentLabel label, bool oneBase) {
	double x = 0;
	string arg;

	if (label != ArgumentLabel::None) {
		// search for label
		arg = getArgument(label);
		x = Util::toDouble(arg);
	} else {
		// find numeric argument
		for (int argumenti = 0; argumenti < arguments.size(); argumenti++) {
			arg = arguments.at(argumenti)->value;
			if (Util::isNumeric(arg)) {
				x = Util::toDouble(arg);
				break;
			}
		}
	}

	if (oneBase) {
		if (x >= 1) {
			x -= 1;
		}
	}

	return x;
}

bool ScriptOperation::getArgumentBoolean(ArgumentLabel label) {
	bool b = false;
	string arg;

	if (label != ArgumentLabel::None) {
		// search for label
		for (Argument* argument : arguments) {
			if (argument->argumentLabel == label) {
				arg = argument->value;
				b = true;
				if (Util::isBoolean(arg)) {
					b = Util::toBoolean(arg);
				}
				break;
			}
		}
	} else {
		// find boolean argument
		for (int argumenti = 0; argumenti < arguments.size(); argumenti++) {
			arg = arguments.at(argumenti)->value;
			if (Util::isBoolean(arg)) {
				b = Util::toBoolean(arg);
				break;
			}
		}
	}
	return b;
}

ClusterDrawMode ScriptOperation::getClusterDrawMode(ClusterDrawMode defaultArgument) {
	int clusterDrawMode = 0;
	int clusterDrawMode0;
	string fullArg = getArgument(ArgumentLabel::DrawMode);
	vector<string> args = Util::split(fullArg, vector<string>{"|", "&", "+"}, true);
	string arg;
	bool ok = (!args.empty());

	for (string arg0 : args) {
		arg = Util::trim_copy(arg0);
		clusterDrawMode0 = Util::getListIndex(ClusterDrawModes, arg);
		if (clusterDrawMode0 >= 0) {
			clusterDrawMode |= clusterDrawMode0;
		} else {
			ok = false;
			throw invalid_argument("Value " + arg + " not valid for DrawMode");
		}
	}

	if (ok) {
		return (ClusterDrawMode)clusterDrawMode;
	}
	return defaultArgument;
}

OperationInfo ScriptOperation::getOperationInfo(ScriptOperationType type) {
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description = "";

	// **************************

	return OperationInfo(requiredArguments, optionalArguments, description);
}

string ScriptOperation::getOperationList() {
	string s;
	ScriptOperationType type;
	OperationInfo* info;
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description;
	bool firstArg;

	// RTF tutorial: http://www.pindari.com/rtf1.html

	s = "{\\rtf1\\ansi\\deff0 {\\fonttbl {\\f0 Consolas;}}\n{\\colortbl;\\red0\\green0\\blue0;\\red127\\green127\\blue127;\\red0\\green0\\blue255;}";
	s += "\\margl720\\margr720\\margt720\\margb720 \\fs20\n";	// 0.5 inch margins; font size 10
	s += "\\b BioImageOperation script operations\\b0\\line\n\\line\n";
	/*
	for (int types : Enum::GetValues(ScriptOperationType::typeid)) {
		firstArg = true;
		type = (ScriptOperationType)types;

		if (type != ScriptOperationType::None) {
			info = getOperationInfo(type);
			requiredArguments = info->requiredArguments;
			optionalArguments = info->optionalArguments;
			description = info->description;

			s += string::Format("\\b {0}\\b0 (", type);

			if (requiredArguments) {
				for (ArgumentLabel arg : requiredArguments) {
					if (!firstArg) {
						s += ", ";
					}
					s += string::Format("{0}", arg);
					firstArg = false;
				}
			}

			if (optionalArguments) {
				for (ArgumentLabel arg : optionalArguments) {
					if (!firstArg) {
						s += ", ";
					}
					s += "{\\cf2 ";
					s += string::Format("{0}", arg);
					s += "}";
					firstArg = false;
				}
			}

			s += ") \\line\n";
			s += "{\\cf3 \\tab \\bullet " + description + "}\\line\n\\line\n";
		}
	}
	s += "\\line\n";
	s += "\\b Arguments:\\b0 \\tab Required \\tab {\\cf2 Optional}\\line\n";
	s += "}";

	s = s->Replace("\n", "\r\n");
	*/
	return s;
}

void ScriptOperation::writeOperationList(string filename) {
	string s = getOperationList();
	ofstream file(filename);
	file << s;
	file.close();
}

bool ScriptOperation::initFrameSource(FrameType frameType, int apiCode, string basepath, string templatePath, string start, string length, double fps0, int interval) {
	bool ok = true;

	if (!frameSource) {
		switch (frameType)
		{
		case FrameType::Image: frameSource = new ImageSource(); break;
		case FrameType::Video: frameSource = new VideoSource(); break;
		case FrameType::Capture: frameSource = new CaptureSource(); break;
		}
		if (frameSource)
		{
			ok = frameSource->init(apiCode, basepath, templatePath, start, length, fps0, interval);
		}
	}
	return ok;
}

void ScriptOperation::initFrameOutput(FrameType frameType, string basepath, string templatePath, string defaultExtension, string start, string length, double fps, string codecs) {
	if (!frameOutput) {
		switch (frameType)
		{
		case FrameType::Image: frameOutput = new ImageOutput(); break;
		case FrameType::Video: frameOutput = new VideoOutput(); break;
		}
		if (frameOutput) {
			frameOutput->init(basepath, templatePath, defaultExtension, start, length, fps, codecs);
		}
	}
}

void ScriptOperation::close() {
	if (innerOperations) {
		innerOperations->close();
	}

	if (frameSource) {
		frameSource->close();
	}

	if (frameOutput) {
		frameOutput->close();
	}
}
