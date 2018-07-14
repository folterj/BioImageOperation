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
#include "FrameOutput.h"
#include <vcclr.h>

#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed
#include "NumericPath.h"

using namespace cv;


/*
 * Output to video file
 */

class VideoOutput : public FrameOutput
{
public:
	VideoWriter videoWriter;
	bool videoIsOpen = false;
	gcroot<NumericPath^> outputPath = gcnew NumericPath();
	int width = 0;
	int height = 0;
	bool isColor = true;
	double fps = 0;
	int codec = 0;

	VideoOutput();
	~VideoOutput();
	void init(System::String^ basePath, System::String^ filePath, System::String^ defaultExtension = "", System::String^ start = "", System::String^ length = "", double fps = 0, System::String^ codecs = "");
	void reset();
	bool open();
	void close();
	bool writeImage(Mat* image);
};
