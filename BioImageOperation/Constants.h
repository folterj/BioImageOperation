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

#include<vector>


/*
 * Constants and general types/enums
 */

public enum class ScriptOperationType
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


public enum class ImageColorMode
{
	GrayScale,
	Color,
	ColorAlpha
};


public enum class Palette
{
	Grayscale,
	Heat,
	Rainbow,
};

public enum class ClusterDrawMode : int
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


public enum class AccumMode
{
	Age,
	Usage
};


public enum class PathDrawMode
{
	Age,
	Usage,
	Usage2,
	Links,
	LinksMove,
};

public enum class FrameType
{
	Image,
	Video,
	Capture
};

public enum class DrawPosition
{
	Full,
	TopLeft,
	BottomLeft,
	TopRight,
	BottomRight,
};


class Constants
{
public:
	static const int statBins = 20;
	static const int nDisplays;
	static const int nInfoWindows;
	static const int nTrackers;
	static const int seekModeInterval;

	static const std::string webPage;
	static const std::string webFilesUrl;
	static const std::string scriptFileDialogFilter;
	static const int defaultScriptFileDialogFilter;
	static const std::string defaultDataExtension;
	static const std::string defaultImageExtension;
	static const std::string defaultVideoExtension;
	static const std::string defaultVideoCodec;

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
