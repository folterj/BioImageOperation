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

#include "ImageOutput.h"
#include "Util.h"

using namespace System::IO;


ImageOutput::ImageOutput()
{
}

ImageOutput::~ImageOutput()
{
	close();
}

void ImageOutput::reset()
{
	outputPath->reset();
	start = 0;
	end = 0;
	filei = 0;
}

void ImageOutput::init(System::String^ basePath, System::String^ filePath, System::String^ defaultExtension, System::String^ start, System::String^ length, double fps0, System::String^ codecs)
{
	reset();
	int lengthi;

	if (!outputPath->setOutputPath(basePath, filePath, defaultExtension))
	{
		throw gcnew IOException(System::String::Format("Unable to write to {0}", Util::extractFilePath(outputPath->initialPath)));
	}

	if (fps0 == 0)
	{
		fps0 = 1;
	}

	this->start = Util::parseFrameTime(start, fps0);
	lengthi = Util::parseFrameTime(length, fps0);

	if (lengthi > 0)
	{
		this->end = this->start + lengthi;
	}
}

bool ImageOutput::writeImage(Mat* image)
{
	if (filei >= start && (filei < end || end == 0))
	{
		Util::saveImage(outputPath->createFilePath(filei), image);
	}
	filei++;
	return true;
}

void ImageOutput::close()
{
}
