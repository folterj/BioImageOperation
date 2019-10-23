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
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "ColorScale.h"

using namespace System;
using namespace cv;


/*
 * General utilities
 */
 
class Util
{
public:
	static double toDouble(System::String^ s);
	static bool isNumeric(System::String^ s);
	static bool isBoolean(System::String^ s);
	static int parseFrameTime(System::String^ s, double fps);
	static double calcDistance(double x0, double y0, double x1, double y1);
	static double calcDistance(double x, double y);

	static double getMomentsAngle(Moments* moments);
	static double calcAngleDif(double angle1, double angle2);

	static Scalar getLabelColor(int label0);
	static Scalar getHeatScale(double scale);
	static Scalar getRainbowScale(double scale);
	static Scalar bgrtoScalar(BGR bgr);

	static std::string stdString(System::String^ s);
	static System::String^ netString(std::string s);
	static std::vector<std::string> stdStringVector(array<System::String^>^ list);
	static System::String^ getExceptionDetail(System::Exception^ e);
	
	static Mat loadImage(System::String^ fileName);
	static void saveImage(System::String^ fileName, Mat* image);
	static bool isValidImage(Mat* image);
	static System::String^ getCodecString(int codec);

	static array<System::String^>^ getImageFileNames(System::String^ searchPath);
	static System::String^ extractFilePath(System::String^ path);
	static System::String^ extractTitle(System::String^ path);
	static System::String^ extractFileName(System::String^ path);
	static System::String^ extractFileTitle(System::String^ fileName);
	static System::String^ extractFileExtension(System::String^ fileName);
	static System::String^ combinePath(System::String^ basePath, System::String^ templatePath);
	static System::String^ getUrl(System::String^ url);
	static bool openWebLink(System::String^ link);
	static int compareVersions(System::String^ version1, System::String^ version2);
};
