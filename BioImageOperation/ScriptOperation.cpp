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

#include "ScriptOperation.h"
#include "ScriptOperations.h"
#include "Util.h"
#include "ImageSource.h"
#include "VideoSource.h"
#include "CaptureSource.h"
#include "ImageOutput.h"
#include "VideoOutput.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace System::IO;


ScriptOperation::ScriptOperation()
{
}

ScriptOperation::ScriptOperation(System::String^ line)
{
	extract(line);
}

ScriptOperation::~ScriptOperation()
{
	if (innerOperations)
	{
		delete innerOperations;
	}

	for (int i = 0; i < arguments.size(); i++)
	{
		delete arguments.at(i);
	}

	if (frameSource)
	{
		delete frameSource;
	}

	if (frameOutput)
	{
		delete frameOutput;
	}
}

void ScriptOperation::reset()
{
	if (innerOperations)
	{
		innerOperations->reset();
	}
}

void ScriptOperation::extract(System::String^ line)
{
	System::String^ operation;
	int i, i1, i2;
	int interval0, offset0;

	this->line = Util::stdString(line);

	i = line->IndexOf(":");
	i1 = line->IndexOf("-");
	i2 = line->IndexOf("(");
	if (i > 0 && (i < i2 || i2 < 0))
	{
		if (i1 > 0 && i1 < i)
		{
			// interval & offset
			if (int::TryParse(line->Substring(0, i1), interval0))
			{
				interval = interval0;
			}
			i1++;
			if (int::TryParse(line->Substring(i1, i - i1), offset0))
			{
				offset = offset0;
			}
		}
		else if (int::TryParse(line->Substring(0, i), interval0))
		{
			// interval
			interval = interval0;
		}
		line = line->Substring(i + 1)->Trim();
	}

	i = line->IndexOf("=");
	i1 = line->IndexOf("(");
	if (i > 0 && i < i1)
	{
		asignee = Util::stdString(line->Substring(0, i)->Trim());
		line = line->Substring(i + 1)->Trim();
	}

	i1 = line->IndexOf("(");	// get index again in shortened line
	if (i1 > 0)
	{
		operation = line->Substring(0, i1)->Trim();
		i2 = line->LastIndexOf(")");
		if (i2 < 0)
		{
			i2 = line->Length;
		}
		line = line->Substring(i1 + 1, i2 - i1 - 1)->Trim();

		for each(System::String^ arg in line->Split(','))
		{
			if (arg->Trim() != "")
			{
				arguments.push_back(new Argument(arg->Trim()));
			}
		}
	}

	if (Enum::TryParse<ScriptOperationType>(operation, true, operationType))
	{
		checkArguments();
	}
	else
	{
		throw gcnew ArgumentException("Unkown operation: " + operation);
	}
}

void ScriptOperation::checkArguments()
{
	OperationInfo^ info = getOperationInfo(operationType);
	array<ArgumentLabel>^ requiredArguments = info->requiredArguments;
	array<ArgumentLabel>^ optionalArguments = info->optionalArguments;
	bool found;

	if (requiredArguments && optionalArguments)
	{
		// check required arguments
		for each (ArgumentLabel label in requiredArguments)
		{
			found = false;
			for each (Argument* argument in arguments)
			{
				if (argument->argumentLabel != ArgumentLabel::None)
				{
					if (argument->argumentLabel == label)
					{
						found = true;
					}
				}
				else if (requiredArguments->Length == 1 && arguments.size() == 1)
				{
					if (label != ArgumentLabel::Path)
					{
						// exception: if only single required argument and no label used (except for path as this should be found)
						found = true;
					}
				}
			}
			if (!found)
			{
				throw gcnew ArgumentException("Missing required argument: " + label.ToString());
			}
		}

		// check all arguments
		for each (Argument* argument in arguments)
		{
			// ignore non-labelled arguments
			if (argument->argumentLabel != ArgumentLabel::None)
			{
				found = false;
				for each (ArgumentLabel label in requiredArguments)
				{
					if (label == argument->argumentLabel)
					{
						found = true;
					}
				}
				for each (ArgumentLabel label in optionalArguments)
				{
					if (label == argument->argumentLabel)
					{
						found = true;
					}
				}
				if (!found)
				{
					throw gcnew ArgumentException("Unexpected argument: " + Util::netString(argument->allArgument));
				}
			}
		}
	}
}

bool ScriptOperation::hasInnerOperations()
{
	if (innerOperations != NULL)
	{
		return innerOperations->hasOperations();
	}
	return false;
}

ScriptOperation* ScriptOperation::getNextInnerOperation()
{
	ScriptOperation* innerOperation = NULL;
	if (hasInnerOperations())
	{
		innerOperation = innerOperations->getCurrentOperation();
		innerOperations->moveNextOperation();
	}
	return innerOperation;
}

System::String^ ScriptOperation::getArgument(ArgumentLabel label)
{
	System::String^ arg = "";

	if (label != ArgumentLabel::None)
	{
		// search for label
		for each(Argument* argument in arguments)
		{
			if (argument->argumentLabel == label)
			{
				arg = Util::netString(argument->value);
				break;
			}
		}
	}
	else if (arguments.size() > 0)
	{
		// else return first argument
		arg = Util::netString(arguments.at(0)->value);
	}
	
	return arg;
}

double ScriptOperation::getArgumentNumeric(ArgumentLabel label, bool oneBase)
{
	double x = 0;
	System::String^ arg;

	if (label != ArgumentLabel::None)
	{
		// search for label
		arg = getArgument(label);
		x = Util::toDouble(arg);
	}
	else
	{
		// find numeric argument
		for (int argumenti = 0; argumenti < arguments.size(); argumenti++)
		{
			arg = Util::netString(arguments.at(argumenti)->value);
			if (Util::isNumeric(arg))
			{
				x = Util::toDouble(arg);
				break;
			}
		}
	}

	if (oneBase)
	{
		if (x >= 1)
		{
			x -= 1;
		}
	}

	return x;
}

bool ScriptOperation::getArgumentBoolean(ArgumentLabel label)
{
	bool b = false;
	System::String^ arg;

	if (label != ArgumentLabel::None)
	{
		// search for label
		for each(Argument* argument in arguments)
		{
			if (argument->argumentLabel == label)
			{
				arg = Util::netString(argument->value);
				b = true;
				if (Util::isBoolean(arg))
				{
					b = bool::Parse(arg);
				}
				break;
			}
		}
	}
	else
	{
		// find boolean argument
		for (int argumenti = 0; argumenti < arguments.size(); argumenti++)
		{
			arg = Util::netString(arguments.at(argumenti)->value);
			if (Util::isBoolean(arg))
			{
				b = bool::Parse(arg);
				break;
			}
		}
	}
	return b;
}

generic<class type> type ScriptOperation::getArgument(ArgumentLabel label, type defaultArgument)
{
	System::String^ arg = getArgument(label);
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

ClusterDrawMode ScriptOperation::getClusterDrawMode(ClusterDrawMode defaultArgument)
{
	int cluterDrawMode = 0;
	ClusterDrawMode clusterDrawMode0;
	System::String^ fullArg = getArgument(ArgumentLabel::DrawMode);
	array<System::String^>^ args = fullArg->Split(gcnew array<System::String^>{"|", "&", "+"}, StringSplitOptions::RemoveEmptyEntries);
	System::String^ arg;
	bool ok = (args->Length != 0);

	for each (System::String^ arg0 in args)
	{
		arg = arg0->Trim();
		if (Enum::TryParse<ClusterDrawMode>(arg, true, clusterDrawMode0))
		{
			cluterDrawMode |= (int)clusterDrawMode0;
		}
		else
		{
			ok = false;
			throw gcnew ArgumentException(System::String::Format("Value {0} not valid for DrawMode", arg));
		}
	}

	if (ok)
	{
		return (ClusterDrawMode)cluterDrawMode;
	}
	return defaultArgument;
}

OperationInfo^ ScriptOperation::getOperationInfo(ScriptOperationType type)
{
	array<ArgumentLabel>^ requiredArguments = nullptr;
	array<ArgumentLabel>^ optionalArguments = nullptr;
	System::String^ description = "";

	switch (type)
	{
	case ScriptOperationType::SetPath:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Set path for relative file paths (by default path of current script file)";
		break;
	}

	case ScriptOperationType::CreateImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::ColorMode, ArgumentLabel::Red, ArgumentLabel::Green, ArgumentLabel::Blue };
		description = "Create a new image";
		break;
	}

	case ScriptOperationType::OpenImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Interval };
		description = "Open image file(s) for processing, accepts file name pattern, start/length #";
		break;
	}

	case ScriptOperationType::OpenVideo:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Interval };
		description = "Open video file(s) and process frames, accepts file name pattern, start/length frames or time (ffmpeg formats supported) (API: See OpenCV API codes)";
		break;
	}

	case ScriptOperationType::OpenCapture:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::API, ArgumentLabel::Path, ArgumentLabel::Source, ArgumentLabel::Width, ArgumentLabel::Height, ArgumentLabel::Interval };
		description = "Open capturing from video (IP) path or camera source (#) (API: See OpenCV API codes)";
		break;
	}

	case ScriptOperationType::SaveImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length };
		description = "Save image to file, start/length frames or time";
		break;
	}

	case ScriptOperationType::SaveVideo:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Start, ArgumentLabel::Length, ArgumentLabel::Fps, ArgumentLabel::Codec };
		description = "Create video file and save image to video file, start/length frames or time, using fourcc codec string (supports installed encoders)";
		break;
	}

	case ScriptOperationType::ShowImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display };
		description = "Show image on screen (low priority screen updates) (display: 1 - 4)";
		break;
	}

	case ScriptOperationType::StoreImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Store current image in memory";
		break;
	}

	case ScriptOperationType::GetImage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Get specified stored image from memory";
		break;
	}

	case ScriptOperationType::Grayscale:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to gray scale";
		break;
	}

	case ScriptOperationType::Color:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to color";
		break;
	}

	case ScriptOperationType::ColorAlpha:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Convert image to color with alpha channel";
		break;
	}

	case ScriptOperationType::GetSaturation:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract saturation from image";
		break;
	}

	case ScriptOperationType::GetHsValue:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract (HSV) Value from image";
		break;
	}

	case ScriptOperationType::GetHsLightness:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Extract (HSL) Lightness from image";
		break;
	}

	case ScriptOperationType::Scale:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Width, ArgumentLabel::Height };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Scale image";
		break;
	}

	case ScriptOperationType::Crop:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::X, ArgumentLabel::Y, ArgumentLabel::Width, ArgumentLabel::Height };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Crop image";
		break;
	}

	case ScriptOperationType::Mask:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Perform mask on current image";
		break;
	}

	case ScriptOperationType::Threshold:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Level };
		description = "Convert image to binary using threshold level, or in case not provided using automatic Otsu method";
		break;
	}

	case ScriptOperationType::Difference:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Perform difference of current image and specified image";
		break;
	}

	case ScriptOperationType::DifferenceAbs:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Perform absolute difference of current image and specified image";
		break;
	}

	case ScriptOperationType::Add:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Adds specified image to current image";
		break;
	}

	case ScriptOperationType::Multiply:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Factor };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Perform multiplication of all color channels by specified factor";
		break;
	}

	case ScriptOperationType::Invert:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label };
		description = "Invert image";
		break;
	}

	case ScriptOperationType::UpdateBackground:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Weight };
		description = "Add image to the adaptive background buffer";
		break;
	}

	case ScriptOperationType::UpdateAverage:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Weight };
		description = "Add image to the average buffer";
		break;
	}

	case ScriptOperationType::AddSeries:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Maximum };
		description = "Add image to image series buffer";
		break;
	}

	case ScriptOperationType::GetSeriesMedian:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Retrieve median image of image series buffer";
		break;
	}

	case ScriptOperationType::AddAccum:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::AccumMode };
		description = "Add image to the accumulative buffer (AccumMode: Age, Usage)";
		break;
	}

	case ScriptOperationType::GetAccum:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Retrieve the accumulative buffer and convert to image (Palette: Grayscale, Heat, Rainbow)";
		break;
	}

	case ScriptOperationType::CreateClusters:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MinArea, ArgumentLabel::MaxArea };
		description = "Create clusters; auto calibrate using initial images if no parameters specified";
		break;
	}

	case ScriptOperationType::CreateTracks:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::MaxMove, ArgumentLabel::MinActive, ArgumentLabel::MaxInactive };
		description = "Create cluster tracking; auto calibrate using initial images if no parameters specified";
		break;
	}

	case ScriptOperationType::CreatePaths:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Distance };
		description = "Create common path usage";
		break;
	}

	case ScriptOperationType::DrawClusters:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw clusters (DrawMode: Point|Circle|Box|Angle|Label|Labeln|Fill)";
		break;
	}

	case ScriptOperationType::DrawTracks:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::DrawMode };
		description = "Draw tracked clusters (DrawMode: Point|Circle|Box|Angle|Label|Labeln|Track|Tracks)";
		break;
	}

	case ScriptOperationType::DrawPaths:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker, ArgumentLabel::PathDrawMode, ArgumentLabel::Power, ArgumentLabel::Palette };
		description = "Draw common paths (PathDrawMode: Age, Usage, Links, LinksMove) (Palette: Grayscale, Heat, Rainbow)";
		break;
	}

	case ScriptOperationType::DrawTrackInfo:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Tracker };
		description = "Draw tracking stats on image";
		break;
	}

	case ScriptOperationType::SaveClusters:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save clusters to CSV file (Format: ByTime, ByLabel, Split)";
		break;
	}

	case ScriptOperationType::SaveTracks:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Format, ArgumentLabel::Contour };
		description = "Save cluster tracking to CSV file (Format: ByTime, ByLabel, Split)";
		break;
	}

	case ScriptOperationType::SavePaths:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save paths to CSV file";
		break;
	}

	case ScriptOperationType::ShowTrackInfo:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker, ArgumentLabel::Display };
		description = "Show tracking information on screen (Display: 1 - 4)";
		break;
	}

	case ScriptOperationType::SaveTrackInfo:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save tracking information to CSV file";
		break;
	}

	case ScriptOperationType::SaveTrackLog:
	{
		requiredArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Path };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Tracker };
		description = "Save tracking log to CSV file";
		break;
	}

	case ScriptOperationType::DrawLegend:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::Label, ArgumentLabel::Display, ArgumentLabel::Position };
		description = "Draw legend (Display: 1 - 4 or Position on image: TopLeft, BottomLeft, TopRight, BottomRight)";
		break;
	}

	case ScriptOperationType::Wait:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { ArgumentLabel::MS };
		description = "Pause execution for a period (1000 ms default)";
		break;
	}

	case ScriptOperationType::Debug:
	{
		requiredArguments = gcnew array<ArgumentLabel> { };
		optionalArguments = gcnew array<ArgumentLabel> { };
		description = "Debug mode";
		break;
	}

	// end of switch
	}

	return gcnew OperationInfo(requiredArguments, optionalArguments, description);
}

System::String^ ScriptOperation::getOperationList()
{
	System::String^ s;
	ScriptOperationType type;
	OperationInfo^ info;
	array<ArgumentLabel>^ requiredArguments;
	array<ArgumentLabel>^ optionalArguments;
	System::String^ description;
	bool firstArg;

	// RTF tutorial: http://www.pindari.com/rtf1.html

	s = "{\\rtf1\\ansi\\deff0 {\\fonttbl {\\f0 Consolas;}}\n{\\colortbl;\\red0\\green0\\blue0;\\red127\\green127\\blue127;\\red0\\green0\\blue255;}";
	s += "\\margl720\\margr720\\margt720\\margb720 \\fs20\n";	// 0.5 inch margins; font size 10
	s += "\\b BioImageOperation script operations\\b0\\line\n\\line\n";

	for each (int types in Enum::GetValues(ScriptOperationType::typeid))
	{
		firstArg = true;
		type = (ScriptOperationType)types;

		if (type != ScriptOperationType::None)
		{
			info = getOperationInfo(type);
			requiredArguments = info->requiredArguments;
			optionalArguments = info->optionalArguments;
			description = info->description;

			s += System::String::Format("\\b {0}\\b0 (", type);

			if (requiredArguments)
			{
				for each (ArgumentLabel arg in requiredArguments)
				{
					if (!firstArg)
					{
						s += ", ";
					}
					s += System::String::Format("{0}", arg);
					firstArg = false;
				}
			}

			if (optionalArguments)
			{
				for each (ArgumentLabel arg in optionalArguments)
				{
					if (!firstArg)
					{
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

void ScriptOperation::writeOperationList(System::String^ filename)
{
	System::String^ s = getOperationList();
	File::WriteAllText(filename, s);
}

bool ScriptOperation::initFrameSource(FrameType frameType, int apiCode, System::String^ basePath, System::String^ templatePath, System::String^ start, System::String^ length, double fps0, int interval)
{
	bool ok = true;

	if (!frameSource)
	{
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

void ScriptOperation::initFrameOutput(FrameType frameType, System::String^ basePath, System::String^ templatePath, System::String^ defaultExtension, System::String^ start, System::String^ length, double fps, System::String^ codecs)
{
	if (!frameOutput)
	{
		switch (frameType)
		{
		case FrameType::Image: frameOutput = new ImageOutput(); break;
		case FrameType::Video: frameOutput = new VideoOutput(); break;
		}
		if (frameOutput)
		{
			frameOutput->init(basePath, templatePath, defaultExtension, start, length, fps, codecs);
		}
	}
}

void ScriptOperation::close()
{
	if (innerOperations)
	{
		innerOperations->close();
	}

	if (frameSource)
	{
		frameSource->close();
	}

	if (frameOutput)
	{
		frameOutput->close();
	}
}
