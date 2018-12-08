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

using namespace System;
using namespace cv;


/*
 * Base class for image source classes
 */

class FrameSource abstract
{
public:
	virtual void reset() = 0;
	virtual bool init(int apiCode, System::String^ basePath, System::String^ filePath, System::String^ start = "", System::String^ length = "", double fps0 = 1, int interval = 1) = 0;
	virtual bool getNextImage(Mat* image) = 0;
	virtual void close() = 0;

	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual double getFps() = 0;

	virtual System::String^ getLabel() = 0;
	virtual int getCurrentFrame() = 0;
	virtual int getTotalFrames() = 0;
};
