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
	IsColor,
	AccumMode,
	DrawMode,
	DrawLabel,
	MS,
	Label,
	Start,
	Length,
	Interval,
	Maximum,
	Level,
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
	Contour
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
	"IsColor",
	"AccumMode",
	"DrawMode",
	"DrawLabel",
	"MS",
	"Label",
	"Start",
	"Length",
	"Interval",
	"Maximum",
	"Level",
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
	"Contour"
};

class Argument
{
public:
	string allArgument;
	ArgumentLabel argumentLabel = ArgumentLabel::None;
	string value = "";
	bool required = false;

	Argument(string arg);
	ArgumentLabel getArgumentLabel(string arg);
};
