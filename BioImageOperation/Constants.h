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
	SetPath,

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
	GetSaturation,
	GetHsValue,
	GetHsLightness,

	Scale,
	Crop,
	Mask,
	Threshold,
	Difference,
	DifferenceAbs,
	Add,
	Multiply,
	Invert,

	UpdateBackground,
	UpdateAverage,
	ClearSeries,
	AddSeries,
	GetSeriesMedian,
	AddAccum,
	GetAccum,

	CreateClusters,
	CreateTracks,
	CreatePaths,
	DrawClusters,
	DrawTracks,
	DrawPaths,
	SaveClusters,
	SaveTracks,
	SavePaths,

	ShowTrackInfo,
	DrawTrackInfo,
	SaveTrackInfo,
	SaveTrackLog,
	DrawLegend,

	Wait,
	Debug,
};

const vector<string> ScriptOperationTypes =
{
	"None",
	"SetPath",

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
	"GetSaturation",
	"GetHsValue",
	"GetHsLightness",

	"Scale",
	"Crop",
	"Mask",
	"Threshold",
	"Difference",
	"DifferenceAbs",
	"Add",
	"Multiply",
	"Invert",

	"UpdateBackground",
	"UpdateAverage",
	"ClearSeries",
	"AddSeries",
	"GetSeriesMedian",
	"AddAccum",
	"GetAccum",

	"CreateClusters",
	"CreateTracks",
	"CreatePaths",
	"DrawClusters",
	"DrawTracks",
	"DrawPaths",
	"SaveClusters",
	"SaveTracks",
	"SavePaths",

	"ShowTrackInfo",
	"DrawTrackInfo",
	"SaveTrackInfo",
	"SaveTrackLog",
	"DrawLegend",

	"Wait",
	"Debug"
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

enum class ImageColorMode
{
	GrayScale,
	Color,
	ColorAlpha
};

enum class Palette
{
	Grayscale,
	Heat,
	Rainbow,
};

enum class ClusterDrawMode : int
{
	None = 0,
	Point = 0x01,
	Circle = 0x02,
	Box = 0x04,
	Angle = 0x08,
	Label = 0x10,
	Labeln = 0x20,
	Track = 0x40,
	Tracks = 0x80,
	Fill = 0x100,
	ClusterDefault = Fill,
	TracksDefault = Tracks,
};

const vector<string> ClusterDrawModes =
{
	"None",
	"Point",
	"Circle",
	"Box",
	"Angle",
	"Label",
	"Labeln",
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


enum class PathDrawMode
{
	Age,
	Usage,
	Usage2,
	Links,
	LinksMove,
};

enum class DrawPosition
{
	Full,
	TopLeft,
	BottomLeft,
	TopRight,
	BottomRight,
};

const vector<string> argumentValues =
{
	"Image",
	"Video",
	"Capture",

	"ByTime",
	"ByLabel",
	"Split",

	"GrayScale",
	"Color",
	"ColorAlpha",

	"Grayscale",
	"Heat",
	"Rainbow",

	"None",
	"Point",
	"Circle",
	"Box",
	"Angle",
	"Label",
	"Labeln",
	"Track",
	"Tracks",
	"Fill",
	"ClusterDefault",
	"TracksDefault",

	"Age",
	"Usage",
	"Usage2",
	"Links",
	"LinksMove",

	"Full",
	"TopLeft",
	"BottomLeft",
	"TopRight",
	"BottomRight"
};

class Constants
{
public:
	static const int statBins = 20;
	static const int nDisplays;
	static const int nInfoWindows;
	static const int nTrackers;
	static const int seekModeInterval;

	static const string webPage;
	static const string webFilesUrl;
	static const string scriptFileDialogFilter;
	static const int defaultScriptFileDialogFilter;
	static const string defaultDataExtension;
	static const string defaultImageExtension;
	static const string defaultVideoExtension;
	static const string defaultVideoCodec;

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
