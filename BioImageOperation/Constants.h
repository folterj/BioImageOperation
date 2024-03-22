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
 * Constants and general types/enums
 */

enum class ScriptOperationType
{
	None,
	Set,
	SetPath,

	Source,
	CreateImage,
	OpenImage,
	OpenVideo,
	OpenCapture,
	SaveImage,
	SaveVideo,
	ShowImage,

	StoreImage,
	GetImage,

	Grayscale,
	Color,
	ColorAlpha,
	Int,
	Float,
	GetHue,
	GetSaturation,
	GetHsValue,
	GetHsLightness,

	Scale,
	Crop,
	Mask,
	Threshold,
	InRangeHsv,
	Erode,
	Dilate,
	Difference,
	DifferenceAbs,
	Add,
	Multiply,
	Invert,

	SetBackground,
	UpdateBackground,
	UpdateWeight,
	UpdateMin,
	UpdateMax,
	ClearSeries,
	AddSeries,
	GetSeriesMedian,
	GetSeriesMean,
	AddAccum,
	GetAccum,

	OpticalCalibration,
	OpticalCorrection,

	CreateClusters,
	CreateTracks,
	CreatePaths,
	DrawClusters,
	DrawTracks,
	DrawPaths,
	DrawTrackCount,
	SaveClusters,
	SaveTracks,
	SavePaths,

	ShowTrackInfo,
	SaveTrackInfo,
	DrawLegend,

	Wait,
	Pause,
	Benchmark,
};

const vector<string> ScriptOperationTypes =
{
	"None",
	"Set",
	"SetPath",

	"Source",
	"CreateImage",
	"OpenImage",
	"OpenVideo",
	"OpenCapture",
	"SaveImage",
	"SaveVideo",
	"ShowImage",

	"StoreImage",
	"GetImage",

	"Grayscale",
	"Color",
	"ColorAlpha",
	"Int",
	"Float",
	"GetHue",
	"GetSaturation",
	"GetHsValue",
	"GetHsLightness",

	"Scale",
	"Crop",
	"Mask",
	"Threshold",
	"InRangeHsv",
	"Erode",
	"Dilate",
	"Difference",
	"DifferenceAbs",
	"Add",
	"Multiply",
	"Invert",

	"SetBackground",
	"UpdateBackground",
	"UpdateWeight",
	"UpdateMin",
	"UpdateMax",
	"ClearSeries",
	"AddSeries",
	"GetSeriesMedian",
	"GetSeriesMean",
	"AddAccum",
	"GetAccum",

	"OpticalCalibration",
	"OpticalCorrection",

	"CreateClusters",
	"CreateTracks",
	"CreatePaths",
	"DrawClusters",
	"DrawTracks",
	"DrawPaths",
	"DrawTrackCount",
	"SaveClusters",
	"SaveTracks",
	"SavePaths",

	"ShowTrackInfo",
	"SaveTrackInfo",
	"DrawLegend",

	"Wait",
	"Pause",
	"Benchmark",
};

enum class OperationMode
{
	Idle,
	Run,
	RequestPause,
	Pause,
	Abort
};

enum class MessageLevel
{
	Info,
	Warning,
	Error
};

const vector<string> MessageLevels =
{
	"Info",
	"Warning",
	"Error"
};

enum class FrameType
{
	Image,
	Video,
	Capture
};

enum class SaveFormat
{
	ByTime,
	ByLabel,
	Split
};

const vector<string> SaveFormats =
{
	"ByTime",
	"ByLabel",
	"Split"
};

enum class ImageColorMode
{
	GrayScale,
	Color,
	ColorAlpha
};

const vector<string> ImageColorModes =
{
	"GrayScale",
	"Color",
	"ColorAlpha"
};

enum class Palette
{
	Grayscale,
	Heat,
	Rainbow
};

const vector<string> Palettes =
{
	"GrayScale",
	"Heat",
	"Rainbow"
};

enum class MedianMode
{
	Normal,
	Light,
	Dark
};

const vector<string> MedianModes =
{
	"Normal",
	"Light",
	"Dark"
};

enum class ClusterDrawMode : int
{
	None = 0,
	Point = 0x1,
	Circle = 0x2,
	Ellipse = 0x4,
	Box = 0x8,
	Angle = 0x10,
	Label = 0x20,
	LabelArea = 0x40,
	LabelLength = 0x80,
	LabelAngle = 0x100,
	Track = 0x200,
	Tracks = 0x400,
	Fill = 0x800,
	ClusterDefault = Fill|Label,
	TracksDefault = Point|Label
};

const vector<string> ClusterDrawModes =
{
	"None",
	"Point",
	"Circle",
	"Ellipse",
	"Box",
	"Angle",
	"Label",
	"LabelArea",
	"LabelLength",
	"LabelAngle",
	"Track",
	"Tracks",
	"Fill",
	"ClusterDefault",
	"TracksDefault"
};

enum class AccumMode
{
	Age,
	Usage
};

const vector<string> AccumModes =
{
	"Age",
	"Usage"
};

enum class PathDrawMode
{
	Age,
	Usage,
	Usage2,
	Usage3,
	Links,
	LinksMove
};

const vector<string> PathDrawModes =
{
	"Age",
	"Usage",
	"Usage2",
	"Usage3",
	"Links",
	"LinksMove"
};

enum class DrawPosition
{
	Full,
	TopLeft,
	BottomLeft,
	TopRight,
	BottomRight
};

const vector<string> DrawPositions =
{
	"Full",
	"TopLeft",
	"BottomLeft",
	"TopRight",
	"BottomRight"
};

enum class TrackingMethod
{
	Any,
	Greedy,
	Hungarian
};


class Constants
{
public:
	static const int statBins = 20;
	static const int nDisplays = 4;
	static const int nTextWindows = 4;
	static const int maxLogBuffer = 1000000;

	static const string defaultScriptExtension;
	static const string defaultHelpExtension;
	static const string defaultDataExtension;
	static const string defaultImageExtension;
	static const string defaultVideoExtension;
	static const string defaultVideoCodec;
	static const string scriptFileDialogFilter;
	static const string scriptHelpDialogFilter;
	static const int defaultScriptFileDialogFilter;
	static const string scriptHelpDefaultFilename;
	static const string webPage;
	static const string webFilesUrl;

	static const string scriptTemplatePath;
	static const string trackingScriptTemplate;
	static const string filenameTemplate;

	static const int seekModeInterval;
	static const int minPixels;
	static const int maxMergedBlobs;
	static const int minPathDistance;

	static const int clusterTrainingCycles;
	static const int trackTrainingCycles;
	static const int trainingDataPoints;
	static const int defMinActive;
	static const int defMaxInactive;
	static const int defMaxMove;
	static const double maxBinaryPixelsFactor;
};
