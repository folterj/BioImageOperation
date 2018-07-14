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
#include "NumericPath.h"


/*
 * Output for image file(s)
 */

class ImageOutput : public FrameOutput
{
public:
	gcroot<NumericPath^> outputPath = gcnew NumericPath();
	int start;
	int end;
	int filei = 0;

	ImageOutput();
	~ImageOutput();
	void reset();
	void init(System::String^ basePath, System::String^ filePath, System::String^ defaultExtension = "", System::String^ start = "", System::String^ length = "", double fps0 = 1, System::String^ codecs = "");
	bool writeImage(Mat* image);
	void close();
};
