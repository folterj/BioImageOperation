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
#include "FrameSource.h"
#include <vcclr.h>
#include "NumericPath.h"

#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed


/*
 * Source for image file(s)
 */

class ImageSource : public FrameSource
{
public:
	gcroot<NumericPath^> sourcePath = gcnew NumericPath();
	gcroot<System::String^> label = "";
	int start = 0;
	int end = 0;
	int interval = 1;
	int width = 0;
	int height = 0;
	int nfiles = 0;
	int filei = 0;

	ImageSource();
	~ImageSource();
	void reset();
	bool init(int apiCode, System::String^ basePath, System::String^ filePath, System::String^ start = "", System::String^ length = "", double fps0 = 1, int interval = 1);
	bool open();
	bool getNextImage(Mat* image);
	void close();

	int getWidth();
	int getHeight();
	double getFps();

	System::String^ getLabel();
	int getCurrentFrame();
	int getTotalFrames();
};
