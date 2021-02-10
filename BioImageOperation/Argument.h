/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <string>
#include <vector>

using namespace std;


/*
 * All labels for script operation arguments
 */

enum class ArgumentType
{
	None,
	Bool,
	Num,
	Fraction,
	TimeFrame,
	Label,
	Display,
	Tracker,
	Path,
	Codec,
	ColorMode,
	AccumMode,
	Palette,
	DrawMode,
	PathDrawMode,
	Format,
	Position,
};

enum class ArgumentLabel
{
	None,
	Source,
	X,
	Y,
	Width,
	Height,
	ColorMode,
	Red,
	Green,
	Blue,
	API,
	Path,
	Codec,
	Fps,
	PixelSize,
	WindowSize,
	AccumMode,
	DrawMode,
	DrawLabel,
	MS,
	Label,
	Start,
	Length,
	Interval,
	Total,
	Maximum,
	Level,
	Radius,
	Weight,
	Factor,
	Power,
	Palette,
	Tracker,
	MinArea,
	MaxArea,
	PathDrawMode,
	Distance,
	MaxMove,
	MinActive,
	MaxInactive,
	Display,
	Position,
	Format,
	Features,
	Contour,
	Debug
};

const vector<string> ArgumentLabels =
{
	"None",
	"Source",
	"X",
	"Y",
	"Width",
	"Height",
	"ColorMode",
	"Red",
	"Green",
	"Blue",
	"API",
	"Path",
	"Codec",
	"Fps",
	"PixelSize",
	"WindowSize",
	"AccumMode",
	"DrawMode",
	"DrawLabel",
	"MS",
	"Label",
	"Start",
	"Length",
	"Interval",
	"Total",
	"Maximum",
	"Level",
	"Radius",
	"Weight",
	"Factor",
	"Power",
	"Palette",
	"Tracker",
	"MinArea",
	"MaxArea",
	"PathDrawMode",
	"Distance",
	"MaxMove",
	"MinActive",
	"MaxInactive",
	"Display",
	"Position",
	"Format",
	"Features",
	"Contour",
	"Debug"
};

class Argument
{
public:
	string allArgument;
	ArgumentType argumentType = ArgumentType::None;
	ArgumentLabel argumentLabel = ArgumentLabel::None;
	string value = "";
	int valueEnum = -1;
	bool used = false;

	Argument(string arg);
	ArgumentLabel getArgumentLabel(string arg);
	bool parseType(ArgumentType argumentType);
	int parseClusterDrawMode(string value);
};
