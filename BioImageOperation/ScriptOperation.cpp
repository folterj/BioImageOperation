/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ScriptOperation.h"
#include "ScriptOperations.h"

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
	string operation;
	int i, i1, i2;
	int interval0, offset0;

	this->line = line;

	i = line->IndexOf(":");
	i1 = line->IndexOf("-");
	i2 = line->IndexOf("(");
	if (i > 0 && (i < i2 || i2 < 0)) {
		if (i1 > 0 && i1 < i) {
			// interval & offset
			if (int::TryParse(line->Substring(0, i1), interval0)) {
				interval = interval0;
			}
			i1++;
			if (int::TryParse(line->Substring(i1, i - i1), offset0)) {
				offset = offset0;
			}
		} else if (int::TryParse(line->Substring(0, i), interval0)) {
			// interval
			interval = interval0;
		}
		line = line->Substring(i + 1)->Trim();
	}

	i = line->IndexOf("=");
	i1 = line->IndexOf("(");
	if (i > 0 && i < i1) {
		asignee = Util::stdString(line->Substring(0, i)->Trim());
		line = line->Substring(i + 1)->Trim();
	}

	i1 = line->IndexOf("(");	// get index again in shortened line
	if (i1 > 0) {
		operation = line->Substring(0, i1)->Trim();
		i2 = line->LastIndexOf(")");
		if (i2 < 0) {
			i2 = line->Length;
		}
		line = line->Substring(i1 + 1, i2 - i1 - 1)->Trim();

		for each (System::String ^ arg in line->Split(',')) {
			if (arg->Trim() != "") {
				arguments.push_back(new Argument(arg->Trim()));
			}
		}
	}

	if (Enum::TryParse<ScriptOperationType>(operation, true, operationType)) {
		checkArguments();
	} else {
		throw gcnew ArgumentException("Unkown operation: " + operation);
	}
}

void ScriptOperation::checkArguments() {
	OperationInfo^ info = getOperationInfo(operationType);
	array<ArgumentLabel>^ requiredArguments = info->requiredArguments;
	array<ArgumentLabel>^ optionalArguments = info->optionalArguments;
	bool found;

	if (requiredArguments && optionalArguments) {
		// check required arguments
		for each (ArgumentLabel label in requiredArguments) {
			found = false;
			for each (Argument * argument in arguments) {
				if (argument->argumentLabel != ArgumentLabel::None) {
					if (argument->argumentLabel == label) {
						found = true;
					}
				} else if (requiredArguments->Length == 1 && arguments.size() == 1) {
					if (label != ArgumentLabel::Path) {
						// exception: if only single required argument and no label used (except for path as this should be found)
						found = true;
					}
				}
			} if (!found) {
				throw gcnew ArgumentException("Missing required argument: " + label.ToString());
			}
		}

		// check all arguments
		for each (Argument * argument in arguments) {
			// ignore non-labelled arguments
			if (argument->argumentLabel != ArgumentLabel::None) {
				found = false;
				for each (ArgumentLabel label in requiredArguments) {
					if (label == argument->argumentLabel) {
						found = true;
					}
				} for each (ArgumentLabel label in optionalArguments) {
					if (label == argument->argumentLabel) {
						found = true;
					}
				}
				if (!found) {
					throw gcnew ArgumentException("Unexpected argument: " + Util::netString(argument->allArgument));
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
		for each (Argument * argument in arguments) {
			if (argument->argumentLabel == label) {
				arg = Util::netString(argument->value);
				break;
			}
		}
	} else if (arguments.size() > 0) {
		// else return first argument
		arg = Util::netString(arguments.at(0)->value);
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
			arg = Util::netString(arguments.at(argumenti)->value);
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
		for each (Argument * argument in arguments) {
			if (argument->argumentLabel == label) {
				arg = Util::netString(argument->value);
				b = true;
				if (Util::isBoolean(arg)) {
					b = bool::Parse(arg);
				}
				break;
			}
		}
	} else {
		// find boolean argument
		for (int argumenti = 0; argumenti < arguments.size(); argumenti++) {
			arg = Util::netString(arguments.at(argumenti)->value);
			if (Util::isBoolean(arg)) {
				b = bool::Parse(arg);
				break;
			}
		}
	}
	return b;
}

generic<class type> type ScriptOperation::getArgument(ArgumentLabel label, type defaultArgument) {
	string arg = getArgument(label);
	ImageColorMode colorMode;
	Palette palette;
	AccumMode accumMode;
	ClusterDrawMode clusterDrawMode;
	PathDrawMode pathDrawMode;
	DrawPosition drawPosition;
	SaveFormat saveFormat;
	bool ok = false;

	if (arg != "")
	{
		if (type::typeid == ImageColorMode::typeid)
		{
			ok = Enum::TryParse<ImageColorMode>(arg, true, colorMode);
		}
		if (type::typeid == Palette::typeid)
		{
			ok = Enum::TryParse<Palette>(arg, true, palette);
		}
		if (type::typeid == AccumMode::typeid)
		{
			ok = Enum::TryParse<AccumMode>(arg, true, accumMode);
		}
		if (type::typeid == ClusterDrawMode::typeid)
		{
			ok = Enum::TryParse<ClusterDrawMode>(arg, true, clusterDrawMode);
		}
		if (type::typeid == PathDrawMode::typeid)
		{
			ok = Enum::TryParse<PathDrawMode>(arg, true, pathDrawMode);
		}
		if (type::typeid == DrawPosition::typeid)
		{
			ok = Enum::TryParse<DrawPosition>(arg, true, drawPosition);
		}
		if (type::typeid == SaveFormat::typeid)
		{
			ok = Enum::TryParse<SaveFormat>(arg, true, saveFormat);
		}

		if (!ok)
		{
			throw gcnew ArgumentException(System::String::Format("Value {0} not valid for {1}", arg, type::typeid));
		}

		if (type::typeid == ImageColorMode::typeid)
		{
			return (type)colorMode;
		}
		if (type::typeid == Palette::typeid)
		{
			return (type)palette;
		}
		if (type::typeid == AccumMode::typeid)
		{
			return (type)accumMode;
		}
		if (type::typeid == ClusterDrawMode::typeid)
		{
			return (type)clusterDrawMode;
		}
		if (type::typeid == PathDrawMode::typeid)
		{
			return (type)pathDrawMode;
		}
		if (type::typeid == DrawPosition::typeid)
		{
			return (type)drawPosition;
		}
		if (type::typeid == SaveFormat::typeid)
		{
			return (type)saveFormat;
		}
	}
	return defaultArgument;
}

ClusterDrawMode ScriptOperation::getClusterDrawMode(ClusterDrawMode defaultArgument) {
	int cluterDrawMode = 0;
	ClusterDrawMode clusterDrawMode0;
	string fullArg = getArgument(ArgumentLabel::DrawMode);
	array<string>^ args = fullArg->Split(gcnew array<string>{"|", "&", "+"}, StringSplitOptions::RemoveEmptyEntries);
	string arg;
	bool ok = (args->Length != 0);

	for each (System::String ^ arg0 in args) {
		arg = arg0->Trim();
		if (Enum::TryParse<ClusterDrawMode>(arg, true, clusterDrawMode0)) {
			cluterDrawMode |= (int)clusterDrawMode0;
		} else {
			ok = false;
			throw gcnew ArgumentException(System::String::Format("Value {0} not valid for DrawMode", arg));
		}
	}

	if (ok) {
		return (ClusterDrawMode)cluterDrawMode;
	}
	return defaultArgument;
}

OperationInfo* ScriptOperation::getOperationInfo(ScriptOperationType type) {
	return new OperationInfo(requiredArguments, optionalArguments, description);
}

string ScriptOperation::getOperationList() {
	string s;
	ScriptOperationType type;
	OperationInfo^ info;
	array<ArgumentLabel>^ requiredArguments;
	array<ArgumentLabel>^ optionalArguments;
	string description;
	bool firstArg;

	// RTF tutorial: http://www.pindari.com/rtf1.html

	s = "{\\rtf1\\ansi\\deff0 {\\fonttbl {\\f0 Consolas;}}\n{\\colortbl;\\red0\\green0\\blue0;\\red127\\green127\\blue127;\\red0\\green0\\blue255;}";
	s += "\\margl720\\margr720\\margt720\\margb720 \\fs20\n";	// 0.5 inch margins; font size 10
	s += "\\b BioImageOperation script operations\\b0\\line\n\\line\n";

	for each (int types in Enum::GetValues(ScriptOperationType::typeid)) {
		firstArg = true;
		type = (ScriptOperationType)types;

		if (type != ScriptOperationType::None) {
			info = getOperationInfo(type);
			requiredArguments = info->requiredArguments;
			optionalArguments = info->optionalArguments;
			description = info->description;

			s += System::String::Format("\\b {0}\\b0 (", type);

			if (requiredArguments) {
				for each (ArgumentLabel arg in requiredArguments) {
					if (!firstArg) {
						s += ", ";
					}
					s += System::String::Format("{0}", arg);
					firstArg = false;
				}
			}

			if (optionalArguments) {
				for each (ArgumentLabel arg in optionalArguments) {
					if (!firstArg) {
						s += ", ";
					}
					s += "{\\cf2 ";
					s += System::String::Format("{0}", arg);
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

	return s;
}

void ScriptOperation::writeOperationList(string filename) {
	string s = getOperationList();
	File::WriteAllText(filename, s);
}

bool ScriptOperation::initFrameSource(FrameType frameType, int apiCode, string basePath, string templatePath, string start, string length, double fps0, int interval) {
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
			ok = frameSource->init(apiCode, basePath, templatePath, start, length, fps0, interval);
		}
	}
	return ok;
}

void ScriptOperation::initFrameOutput(FrameType frameType, string basePath, string templatePath, string defaultExtension, string start, string length, double fps, string codecs) {
	if (!frameOutput) {
		switch (frameType)
		{
		case FrameType::Image: frameOutput = new ImageOutput(); break;
		case FrameType::Video: frameOutput = new VideoOutput(); break;
		}
		if (frameOutput) {
			frameOutput->init(basePath, templatePath, defaultExtension, start, length, fps, codecs);
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
