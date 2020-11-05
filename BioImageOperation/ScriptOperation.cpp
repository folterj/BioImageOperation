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
#include "Constants.h"
#include "ImageSource.h"
#include "VideoSource.h"
#include "CaptureSource.h"
#include "ImageOutput.h"
#include "VideoOutput.h"
#include "Util.h"
#include "OutputStream.h"


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
		parseArguments();
	} else {
		throw invalid_argument("Unkown operation: " + operation);
	}
}

void ScriptOperation::parseArguments() {
	OperationInfo info = getOperationInfo(operationType);
	vector<ArgumentLabel> requiredArguments = info.requiredArguments;
	vector<ArgumentLabel> optionalArguments = info.optionalArguments;
	Argument* foundArgument;
	ArgumentType expectedType;
	string s;
	bool found;

	if (!requiredArguments.empty() && !optionalArguments.empty()) {
		// check required arguments
		for (ArgumentLabel label : requiredArguments) {
			expectedType = getExpectedArgumentType(label);
			found = false;
			for (Argument* argument : arguments) {
				if (argument->argumentLabel != ArgumentLabel::None) {
					if (argument->argumentLabel == label) {
						foundArgument = argument;
						found = true;
					}
				} else if (requiredArguments.size() == 1 && arguments.size() == 1) {
					if (label != ArgumentLabel::Path) {
						// exception: if only single required argument and no label used (except for path as this should be found)
						foundArgument = argument;
						found = true;
					}
				}
			}
			if (found) {
				foundArgument->valueEnum = foundArgument->parseType(expectedType);
				if (foundArgument->valueEnum < 0) {
					s = "Unexpected value for argument: " + foundArgument->allArgument;
					throw invalid_argument(s);
				}
			} else {
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
				}
				for (ArgumentLabel label : optionalArguments) {
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

int ScriptOperation::getArgument(ArgumentLabel label, int defaultArgument) {
	int argumentValue = -1;

	if (label != ArgumentLabel::None) {
		// search for label
		for (Argument* argument : arguments) {
			if (argument->argumentLabel == label) {
				argumentValue = argument->valueEnum;
				break;
			}
		}
	} else if (arguments.size() > 0) {
		// else return first argument
		argumentValue = arguments.at(0)->valueEnum;
	}
	if (argumentValue < 0) {
		argumentValue = defaultArgument;
	}
	return argumentValue;
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

int ScriptOperation::getClusterDrawMode(ClusterDrawMode defaultArgument) {
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
		return clusterDrawMode;
	}
	return (int)defaultArgument;
}

OperationInfo ScriptOperation::getOperationInfo(ScriptOperationType type) {
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description = "";

	switch (type) {
	case ScriptOperationType::SetPath:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Set path for relative file paths (by default path of current script file)";
		break;

	case ScriptOperationType::CreateImage:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::ColorMode, ArgumentLabel::Red, ArgumentLabel::Green, ArgumentLabel::Blue };
		description = "Create a new image";
		break;

	case ScriptOperationType::OpenImage:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Interval };
		description = "Open image file(s) for processing, accepts file name pattern, start/length #";
		break;

	case ScriptOperationType::OpenVideo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Interval };
		description = "Open video file(s) and process frames, accepts file name pattern, start/length frames or time (ffmpeg formats supported) (API: See OpenCV API codes)";
		break;

	case ScriptOperationType::OpenCapture:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Path, ArgumentLabel::Source, ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Interval };
		description = "Open capturing from video (IP) path or camera source (#) (API: See OpenCV API codes)";
		break;

	case ScriptOperationType::SaveImage:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length };
		description = "Save image to file, start/length frames or time";
		break;

	case ScriptOperationType::SaveVideo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Fps, ArgumentLabel::Codec };
		description = "Create video file and save image to video file, start/length frames or time, using fourcc codec string (supports installed encoders)";
		break;

	case ScriptOperationType::ShowImage:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display };
		description = "Show image on screen (low priority screen updates) (display: 1 - 4)";
		break;

	case ScriptOperationType::StoreImage:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Store current image in memory";
		break;

	case ScriptOperationType::GetImage:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Get specified stored image from memory";
		break;

	case ScriptOperationType::Grayscale:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to gray scale";
		break;

	case ScriptOperationType::Color:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to color";
		break;

	case ScriptOperationType::ColorAlpha:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to color with alpha channel";
		break;

	case ScriptOperationType::GetSaturation:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract saturation from image";
		break;

	case ScriptOperationType::GetHsValue:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract (HSV) Value from image";
		break;

	case ScriptOperationType::GetHsLightness:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract (HSL) Lightness from image";
		break;

	case ScriptOperationType::Scale:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Width, ArgumentLabel::Height };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Scale image";
		break;

	case ScriptOperationType::Crop:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::X, ArgumentLabel::Y, ArgumentLabel::Width, ArgumentLabel::Height };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Crop image";
		break;

	case ScriptOperationType::Mask:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Perform mask on current image";
		break;

	case ScriptOperationType::Threshold:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Level };
		description = "Convert image to binary using threshold level, or in case not provided using automatic Otsu method";
		break;

	case ScriptOperationType::Difference:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Perform difference of current image and specified image";
		break;

	case ScriptOperationType::DifferenceAbs:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Perform absolute difference of current image and specified image";
		break;

	case ScriptOperationType::Add:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Adds specified image to current image";
		break;

	case ScriptOperationType::Multiply:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Factor };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Perform multiplication of all color channels by specified factor";
		break;

	case ScriptOperationType::Invert:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label };
		description = "Invert image";
		break;

	case ScriptOperationType::UpdateBackground:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Weight };
		description = "Add image to the adaptive background buffer";
		break;

	case ScriptOperationType::UpdateAverage:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Weight };
		description = "Add image to the average buffer";
		break;

	case ScriptOperationType::ClearSeries:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Clear image series buffer";
		break;

	case ScriptOperationType::AddSeries:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Maximum };
		description = "Add image to image series buffer";
		break;

	case ScriptOperationType::GetSeriesMedian:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Retrieve median image of image series buffer";
		break;

	case ScriptOperationType::AddAccum:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::AccumMode };
		description = "Add image to the accumulative buffer (AccumMode: Age, Usage)";
		break;

	case ScriptOperationType::GetAccum:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Retrieve the accumulative buffer and convert to image (Palette: Grayscale, Heat, Rainbow)";
		break;

	case ScriptOperationType::CreateClusters:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MinArea, ArgumentLabel::MaxArea };
		description = "Create clusters; auto calibrate using initial images if no parameters specified";
		break;

	case ScriptOperationType::CreateTracks:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MaxMove, ArgumentLabel::MinActive, ArgumentLabel::MaxInactive };
		description = "Create cluster tracking; auto calibrate using initial images if no parameters specified";
		break;

	case ScriptOperationType::CreatePaths:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Distance };
		description = "Create common path usage";
		break;

	case ScriptOperationType::DrawClusters:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw clusters (DrawMode: Point|Circle|Box|Angle|Label|Labeln|Fill)";
		break;

	case ScriptOperationType::DrawTracks:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw tracked clusters (DrawMode: Point|Circle|Box|Angle|Label|Labeln|Track|Tracks)";
		break;

	case ScriptOperationType::DrawPaths:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::PathDrawMode, ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Draw common paths (PathDrawMode: Age, Usage, Links, LinksMove) (Palette: Grayscale, Heat, Rainbow)";
		break;

	case ScriptOperationType::DrawTrackInfo:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker };
		description = "Draw tracking stats on image";
		break;

	case ScriptOperationType::SaveClusters:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save clusters to CSV file (Format: ByTime/ByLabel, Split)";
		break;

	case ScriptOperationType::SaveTracks:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save cluster tracking to CSV file (Format: ByTime/ByLabel, Split)";
		break;

	case ScriptOperationType::SavePaths:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save paths to CSV file";
		break;

	case ScriptOperationType::ShowTrackInfo:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Display };
		description = "Show tracking information on screen (Display: 1 - 4)";
		break;

	case ScriptOperationType::SaveTrackInfo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save tracking information to CSV file";
		break;

	case ScriptOperationType::SaveTrackLog:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save tracking log to CSV file";
		break;

	case ScriptOperationType::DrawLegend:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display, ArgumentLabel::Position };
		description = "Draw legend (Display: 1 - 4 or Position on image: TopLeft, BottomLeft, TopRight, BottomRight)";
		break;

	case ScriptOperationType::Wait:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::MS };
		description = "Pause execution for a period (1000 ms default)";
		break;

	case ScriptOperationType::Debug:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { };
		description = "Debug mode";
		break;

	// end of switch
	}

	return OperationInfo(requiredArguments, optionalArguments, description);
}

ArgumentType ScriptOperation::getExpectedArgumentType(ArgumentLabel argument) {
	ArgumentType type = ArgumentType::None;
	switch (argument) {
	case ArgumentLabel::Path:
		type = ArgumentType::Path;
		break;

	case ArgumentLabel::Label:
		type = ArgumentType::Label;
		break;

	case ArgumentLabel::Tracker:
		type = ArgumentType::Tracker;
		break;

	case ArgumentLabel::Display:
		type = ArgumentType::Display;
		break;

	case ArgumentLabel::Contour:
		type = ArgumentType::Bool;
		break;

	case ArgumentLabel::Red:
	case ArgumentLabel::Green:
	case ArgumentLabel::Blue:
	case ArgumentLabel::Level:
	case ArgumentLabel::Weight:
		type = ArgumentType::Fraction;
		break;

	case ArgumentLabel::Width:
	case ArgumentLabel::Height:
	case ArgumentLabel::X:
	case ArgumentLabel::Y:
	case ArgumentLabel::Interval:
	case ArgumentLabel::MS:
	case ArgumentLabel::Power:
	case ArgumentLabel::Source:
	case ArgumentLabel::API:
	case ArgumentLabel::Fps:
	case ArgumentLabel::Factor:
	case ArgumentLabel::Maximum:
	case ArgumentLabel::MinArea:
	case ArgumentLabel::MaxArea:
	case ArgumentLabel::MaxMove:
	case ArgumentLabel::MinActive:
	case ArgumentLabel::MaxInactive:
	case ArgumentLabel::Distance:
		type = ArgumentType::Num;
		break;

	case ArgumentLabel::Start:
	case ArgumentLabel::Length:
		type = ArgumentType::TimeFrame;
		break;

	case ArgumentLabel::Codec:
		type = ArgumentType::Codec;
		break;

	case ArgumentLabel::ColorMode:
		type = ArgumentType::ColorMode;
		break;

	case ArgumentLabel::AccumMode:
		type = ArgumentType::AccumMode;
		break;

	case ArgumentLabel::Palette:
		type = ArgumentType::Palette;
		break;

	case ArgumentLabel::DrawMode:
		type = ArgumentType::DrawMode;
		break;

	case ArgumentLabel::PathDrawMode:
		type = ArgumentType::PathDrawMode;
		break;

	case ArgumentLabel::Format:
		type = ArgumentType::Format;
		break;

	case ArgumentLabel::Position:
		type = ArgumentType::Position;
		break;

		// end of switch
	}
	return type;
}

string ScriptOperation::getOperationList() {
	string s;
	ScriptOperationType type;
	OperationInfo info;
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description;
	bool firstArg;

	// RTF tutorial: http://www.pindari.com/rtf1.html

	s = "{\\rtf1\\ansi\\deff0 {\\fonttbl {\\f0 Consolas;}}\n{\\colortbl;\\red0\\green0\\blue0;\\red127\\green127\\blue127;\\red0\\green0\\blue255;}";
	s += "\\margl720\\margr720\\margt720\\margb720 \\fs20\n";	// 0.5 inch margins; font size 10
	s += "\\b BioImageOperation script operations\\b0\\line\n\\line\n";
	
	for (string types : ScriptOperationTypes) {
		firstArg = true;
		type = (ScriptOperationType)Util::getListIndex(ScriptOperationTypes, types);
		if (type != ScriptOperationType::None) {
			info = getOperationInfo(type);
			requiredArguments = info.requiredArguments;
			optionalArguments = info.optionalArguments;
			description = info.description;

			s += Util::format("\\b %s\\b0 (", types.c_str());

			for (ArgumentLabel arg : requiredArguments) {
				if (!firstArg) {
					s += ", ";
				}
				s += ArgumentLabels[(int)arg];
				firstArg = false;
			}

			for (ArgumentLabel arg : optionalArguments) {
				if (!firstArg) {
					s += ", ";
				}
				s += "{\\cf2 ";
				s += ArgumentLabels[(int)arg];
				s += "}";
				firstArg = false;
			}

			s += ") \\line\n";
			s += "{\\cf3 \\tab \\bullet " + description + "}\\line\n\\line\n";
		}
	}
	s += "\\line\n";
	s += "\\b Arguments:\\b0 \\tab Required \\tab {\\cf2 Optional}\\line\n";
	s += "}";

	s = Util::replace(s, "\n", "\r\n");
	
	return s;
}

void ScriptOperation::writeOperationList(string filename) {
	string s = getOperationList();
	OutputStream outputStream;
	outputStream.init(filename);
	outputStream.write(s);
	outputStream.closeStream();
}

bool ScriptOperation::initFrameSource(FrameType frameType, int apiCode, string basepath, string templatePath, string start, string length, double fps0, int interval) {
	bool ok = true;

	if (!frameSource) {
		switch (frameType) {
		case FrameType::Image: frameSource = new ImageSource(); break;
		case FrameType::Video: frameSource = new VideoSource(); break;
		case FrameType::Capture: frameSource = new CaptureSource(); break;
		}
		if (frameSource) {
			ok = frameSource->init(apiCode, basepath, templatePath, start, length, fps0, interval);
		}
	}
	return ok;
}

void ScriptOperation::initFrameOutput(FrameType frameType, string basepath, string templatePath, string defaultExtension, string start, string length, double fps, string codecs) {
	if (!frameOutput) {
		switch (frameType) {
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
