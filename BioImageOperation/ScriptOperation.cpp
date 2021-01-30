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
#include "config.h"
#include "Types.h"


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
	start = Clock::now();
}

void ScriptOperation::initialFinish() {
	chrono::duration<double> totalElapsed = Clock::now() - start;
	timeElapsedInits += totalElapsed.count();
	countElapsedInit++;
}

void ScriptOperation::finish() {
	chrono::duration<double> totalElapsed = Clock::now() - start;
	timeElapseds += totalElapsed.count();
	countElapsed++;
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
		line = Util::trim(line.substr(i + 1));
	}

	i = line.find("=");
	i1 = line.find("(");
	if (i > 0 && i < i1) {
		asignee = Util::trim(line.substr(0, i));
		line = Util::trim(line.substr(i + 1));
	}

	i1 = line.find("(");	// get index again in shortened line
	if (i1 > 0) {
		operation = Util::trim(line.substr(0, i1));
		i2 = line.find_last_of(")");
		if (i2 < 0) {
			i2 = line.size();
		}
		line = Util::trim(line.substr(i1 + 1, i2 - i1 - 1));

		for (string arg : Util::split(line, ",")) {
			if (Util::trim(arg) != "") {
				arguments.push_back(new Argument(Util::trim(arg)));
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

void ScriptOperation::resetNextArgument() {
	argumentPos = 0;
	positionalMode = true;
}

Argument* ScriptOperation::getNextArgument(ArgumentLabel label) {
	Argument* argument;
	
	if (positionalMode && argumentPos < arguments.size()) {
		argument = arguments[argumentPos];
		if (label == ArgumentLabel::None || argument->argumentLabel == ArgumentLabel::None || argument->argumentLabel == label) {
			argumentPos++;
			return argument;
		}
	}

	positionalMode = false;

	for (Argument* argument : arguments) {
		if (argument->argumentLabel == label) {
			return argument;
		}
	}

	return nullptr;
}

void ScriptOperation::parseArguments() {
	OperationInfo info = getOperationInfo(operationType);
	vector<ArgumentLabel> requiredArguments = info.requiredArguments;
	vector<ArgumentLabel> optionalArguments = info.optionalArguments;
	Argument* argument;
	ArgumentType expectedType;
	
	resetNextArgument();

	for (ArgumentLabel label : requiredArguments) {
		argument = getNextArgument(label);
		if (argument) {
			if (argument->argumentLabel == ArgumentLabel::None) {
				argument->argumentLabel = label;
			}
			argument->used = true;
		} else {
			throw invalid_argument("Missing required argument: " + ArgumentLabels[(int)label]);
		}
	}

	for (ArgumentLabel label : optionalArguments) {
		argument = getNextArgument(label);
		if (argument) {
			if (argument->argumentLabel == ArgumentLabel::None) {
				argument->argumentLabel = label;
			}
			argument->used = true;
		}
	}

	for (Argument* argument : arguments) {
		if (argument->used) {
			expectedType = getExpectedArgumentType(argument->argumentLabel);
			if (!argument->parseType(expectedType)) {
				throw invalid_argument("Unexpected value for argument: " + argument->allArgument);
			}
		} else {
			throw invalid_argument("Unexpected argument: " + argument->allArgument);
		}
	}
}

bool ScriptOperation::hasInnerOperations() {
	if (innerOperations) {
		return innerOperations->hasOperations();
	}
	return false;
}

ScriptOperation* ScriptOperation::getNextInnerOperation() {
	ScriptOperation* innerOperation = nullptr;
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

OperationInfo ScriptOperation::getOperationInfo(ScriptOperationType type) {
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description = "";

	switch (type) {
	case ScriptOperationType::Set:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Path, ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Fps, ArgumentLabel::PixelSize, ArgumentLabel::WindowSize };
		description = "Set parameters";
		break;

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
		description = "Open image file(s) for processing, accepts file name pattern";
		break;

	case ScriptOperationType::OpenVideo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Interval };
		description = "Open video file(s) and process frames, accepts file name pattern (ffmpeg formats supported)";
		break;

	case ScriptOperationType::OpenCapture:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Path, ArgumentLabel::Source, ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Interval };
		description = "Open capturing from video (IP) path or camera source";
		break;

	case ScriptOperationType::SaveImage:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length };
		description = "Save image to file";
		break;

	case ScriptOperationType::SaveVideo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Fps, ArgumentLabel::Codec };
		description = "Create video file and save image to video file (supports installed encoders)";
		break;

	case ScriptOperationType::ShowImage:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display };
		description = "Show image on screen (low priority screen updates)";
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
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Label };
		description = "Scale image (in pixels, or values between 0 and 1)";
		break;

	case ScriptOperationType::Crop:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::X, ArgumentLabel::Y, ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Label };
		description = "Crop image (in pixels, or values between 0 and 1)";
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

	case ScriptOperationType::Erode:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Radius };
		description = "Apply erode filter (default 3x3 pixels)";
		break;

	case ScriptOperationType::Dilate:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Radius };
		description = "Apply dilate filter (default 3x3 pixels)";
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
		description = "Add image to the accumulative buffer";
		break;

	case ScriptOperationType::GetAccum:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Retrieve the accumulative buffer and convert to image";
		break;

	case ScriptOperationType::CreateClusters:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MinArea, ArgumentLabel::MaxArea, ArgumentLabel::Debug };
		description = "Create clusters; auto calibrate using initial images if no parameters specified";
		break;

	case ScriptOperationType::CreateTracks:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MaxMove, ArgumentLabel::MinActive, ArgumentLabel::MaxInactive, ArgumentLabel::Debug };
		description = "Create cluster tracking; auto calibrate using initial images if no parameters specified";
		break;

	case ScriptOperationType::CreatePaths:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Distance, ArgumentLabel::Debug };
		description = "Create common path usage";
		break;

	case ScriptOperationType::DrawClusters:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw clusters";
		break;

	case ScriptOperationType::DrawTracks:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw tracked clusters";
		break;

	case ScriptOperationType::DrawPaths:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::PathDrawMode, ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Draw common paths";
		break;

	case ScriptOperationType::DrawTrackCount:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker };
		description = "Draw tracking count on image";
		break;

	case ScriptOperationType::SaveClusters:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save clusters to CSV file";
		break;

	case ScriptOperationType::SaveTracks:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save cluster tracking to CSV file";
		break;

	case ScriptOperationType::SavePaths:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save paths to CSV file";
		break;

	case ScriptOperationType::ShowTrackInfo:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Display };
		description = "Show tracking information on screen";
		break;

	case ScriptOperationType::SaveTrackInfo:
		requiredArguments = vector<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save tracking information to CSV file";
		break;

	case ScriptOperationType::DrawLegend:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display, ArgumentLabel::Position };
		description = "Draw legend";
		break;

	case ScriptOperationType::Wait:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { ArgumentLabel::MS };
		description = "Pause execution for a period (1000 ms default)";
		break;

	case ScriptOperationType::Benchmark:
		requiredArguments = vector<ArgumentLabel> { };
		optionalArguments = vector<ArgumentLabel> { };
		description = "For benchmarking/debugging";
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
	case ArgumentLabel::Debug:
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
	case ArgumentLabel::Radius:
	case ArgumentLabel::PixelSize:
	case ArgumentLabel::WindowSize:
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

string ScriptOperation::getArgumentDescription(ArgumentLabel argument) {
	string s;
	switch (argument) {
	case ArgumentLabel::Path:
		s = "File path";
		break;

	case ArgumentLabel::Label:
		s = "Label id";
		break;

	case ArgumentLabel::Tracker:
		s = "Tracker id";
		break;

	case ArgumentLabel::Display:
		s = "Display id";
		break;

	case ArgumentLabel::Contour:
		s = "Extract contours";
		break;

	case ArgumentLabel::Red:
		s = "Red color component";
		break;

	case ArgumentLabel::Green:
		s = "Green color component";
		break;

	case ArgumentLabel::Blue:
		s = "Blue color component";
		break;

	case ArgumentLabel::Level:
		s = "Threshold value";
		break;

	case ArgumentLabel::Weight:
		s = "Weight value";
		break;

	case ArgumentLabel::Width:
		s = "Width in pixels";
		break;

	case ArgumentLabel::Height:
		s = "Height in pixels";
		break;

	case ArgumentLabel::X:
		s = "X position in pixels";
		break;

	case ArgumentLabel::Y:
		s = "Y position in pixels";
		break;

	case ArgumentLabel::Interval:
		s = "Interval in number of frames";
		break;

	case ArgumentLabel::MS:
		s = "Time in milliseconds";
		break;

	case ArgumentLabel::Power:
		s = "Exponential power of value range (1E-[power] ... 1)";
		break;

	case ArgumentLabel::Source:
		s = "Camera source (#)";
		break;

	case ArgumentLabel::API:
		s = "OpenCV API code (See OpenCV API codes)";
		break;

	case ArgumentLabel::Fps:
		s = "Frames per second";
		break;

	case ArgumentLabel::PixelSize:
		s = "Size of a pixel in arbitrary unit";
		break;

	case ArgumentLabel::WindowSize:
		s = "Window size for moving average calculations [s]";
		break;

	case ArgumentLabel::Factor:
		s = "Multiplication factor";
		break;

	case ArgumentLabel::Maximum:
		s = "Maximum number of images to keep";
		break;

	case ArgumentLabel::MinArea:
		s = "Minimum area in number of pixels";
		break;

	case ArgumentLabel::MaxArea:
		s = "Maximum area in number of pixels";
		break;

	case ArgumentLabel::MaxMove:
		s = "Maximum movement distance (single frame)";
		break;

	case ArgumentLabel::MinActive:
		s = "Minimum number of frames being active before state is active";
		break;

	case ArgumentLabel::MaxInactive:
		s = "Maximum number of frames being inactive before state is inactive";
		break;

	case ArgumentLabel::Distance:
		s = "Maximum path distance";
		break;

	case ArgumentLabel::Radius:
		s = "Radius in pixels";
		break;

	case ArgumentLabel::Start:
		s = "Start";
		break;

	case ArgumentLabel::Length:
		s = "Length";
		break;

	case ArgumentLabel::Codec:
		s = "Video encoding codec";
		break;

	case ArgumentLabel::ColorMode:
		s = "Color mode";
		break;

	case ArgumentLabel::AccumMode:
		s = "Accumulation mode";
		break;

	case ArgumentLabel::Palette:
		s = "Palette";
		break;

	case ArgumentLabel::DrawMode:
		s = "(Combination of) draw mode(s)";
		break;

	case ArgumentLabel::PathDrawMode:
		s = "Path draw mode";
		break;

	case ArgumentLabel::Debug:
		s = "Debug mode";
		break;

	case ArgumentLabel::Format:
		s = "Output format";
		break;

	case ArgumentLabel::Position:
		s = "Draw position";
		break;

		// end of switch
	}
	return s;
}

string ScriptOperation::getArgumentTypeDescription(ArgumentType type) {
	// valid type values
	string s;
	switch (type) {
	case ArgumentType::Path:
		s = "\"path\"";
		break;

	case ArgumentType::Label:
		s = "string";
		break;

	case ArgumentType::Tracker:
		s = "string";
		break;

	case ArgumentType::Display:
		s = "number 1 - 4";
		break;

	case ArgumentType::Bool:
		s = "true / false";
		break;

	case ArgumentType::Fraction:
		s = "numeric value between 0 and 1";
		break;

	case ArgumentType::Num:
		s = "numeric value";
		break;

	case ArgumentType::TimeFrame:
		s = "time reference as (hours:)minutes:seconds, or frame number";
		break;

	case ArgumentType::Codec:
		s = "4 character codec reference (FOURCC)";
		break;

	case ArgumentType::ColorMode:
		s = Util::getValueList(ImageColorModes);
		break;

	case ArgumentType::AccumMode:
		s = Util::getValueList(AccumModes);
		break;

	case ArgumentType::Palette:
		s = Util::getValueList(Palettes);
		break;

	case ArgumentType::DrawMode:
		s = Util::getValueList(ClusterDrawModes);
		break;

	case ArgumentType::PathDrawMode:
		s = Util::getValueList(PathDrawModes);
		break;

	case ArgumentType::Format:
		s = Util::getValueList(SaveFormats);
		break;

	case ArgumentType::Position:
		s = Util::getValueList(DrawPositions);
		break;

		// end of switch
	}
	return s;
}

string ScriptOperation::getArgumentFullDescription(ArgumentLabel argument) {
	string s, typeDesc;
	s += " - " + ArgumentLabels[(int)argument] + ":\t " + getArgumentDescription(argument);
	typeDesc = getArgumentTypeDescription(getExpectedArgumentType(argument));
	if (typeDesc != "") {
		s += " (" + typeDesc + ")";
	}
	s += "\n";
	return s;
}

string ScriptOperation::getOperationDescription(bool richFormat, string operation) {
	string s, desc;
	OperationInfo info;
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	bool firstArg = true;

	ScriptOperationType type = (ScriptOperationType)Util::getListIndex(ScriptOperationTypes, operation);
	if (type != ScriptOperationType::None) {
		info = getOperationInfo(type);
		requiredArguments = info.requiredArguments;
		optionalArguments = info.optionalArguments;
		desc = info.description;

		if (richFormat) {
			s += "**" + operation + "**";
		} else {
			s += operation;
		}
		s += " (";

		for (ArgumentLabel arg : requiredArguments) {
			if (!firstArg) {
				s += ", ";
			}
			if (richFormat) {
				s += "**" + ArgumentLabels[(int)arg] + "**";
			} else {
				s += ArgumentLabels[(int)arg] + "*";
			}
			firstArg = false;
		}

		for (ArgumentLabel arg : optionalArguments) {
			if (!firstArg) {
				s += ", ";
			}
			s += ArgumentLabels[(int)arg];
			firstArg = false;
		}
		
		s += ")\n";
		if (richFormat) {
			s += "\n";
		}
		s += desc + "\n";
		if (richFormat) {
			s += "\n";
		}

		for (ArgumentLabel arg : requiredArguments) {
			s += getArgumentFullDescription(arg);
		}
		for (ArgumentLabel arg : optionalArguments) {
			s += getArgumentFullDescription(arg);
		}
		if (richFormat) {
			s += "\n";
		}
	}
	return s;
}

string ScriptOperation::getOperationList(bool richFormat, string operation) {
	string s, desc;
	OperationInfo info;
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;

	if (richFormat) {
		s = "# Bio Image Operation script operations (v" + string(PROJECT_VER) + " / " + string(PROJECT_DESC) + ")\n\n";
	}

	if (operation != "") {
		if (Util::contains(ScriptOperationTypes, operation)) {
			s += getOperationDescription(richFormat, operation);
		} else {
			return "Unkown operation: " + operation;
		}
	} else {
		for (string operation : ScriptOperationTypes) {
			s += getOperationDescription(richFormat, operation);
			s += "\n";
		}
	}
	s += "\n";
	if (richFormat) {
		s += "(**Arguments:** [**required**] [optional])";
	} else {
		s += "(Arguments: [required*] [optional])";
	}
	s += "\n";

	//s = Util::replace(s, "\n", "\r\n");
	
	return s;
}

string ScriptOperation::getOperationListSimple() {
	string s;
	for (string operation : ScriptOperationTypes) {
		if (operation != "None") {
			s += operation + "\n";
		}
	}
	return s;
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

double ScriptOperation::getDuration() {
	double duration = 0;
	if (countElapsed != 0) {
		duration = timeElapseds / countElapsed;
		timeElapseds = 0;
		countElapsed = 0;
	}
	return duration;
}

double ScriptOperation::getDurationInit() {
	double duration = 0;
	if (countElapsedInit != 0) {
		duration = timeElapsedInits / countElapsedInit;
		timeElapsedInits = 0;
		countElapsedInit = 0;
	}
	return duration;
}

string ScriptOperation::getBenchmarking(int level) {
	string s = string(level * 2, ' ') + line;
	string times;
	double duration = getDuration();
	double durationInit = getDurationInit();
	if (duration > 0) {
		if (hasInnerOperations()) {
			times = "(" + Util::formatThousands(round(durationInit * 1000000)) + ") ";
		}
		times += Util::formatThousands(round(duration * 1000000)) + " us";
		int i = 80 - s.length() - times.length();
		if (i < 1) {
			i = 1;
		}
		s += string(i, ' ') + times;
	}
	s += "\n";
	if (hasInnerOperations()) {
		s += innerOperations->getBenchmarking(level + 1);
	}
	return s;
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
