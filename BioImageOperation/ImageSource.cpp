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

#include "ImageSource.h"
#include "Util.h"


ImageSource::ImageSource()
{
}

ImageSource::~ImageSource()
{
	close();
}

void ImageSource::reset()
{
	sourcePath->reset();
	nfiles = 0;
	filei = 0;
	start = 0;
	end = 0;
	interval = 1;
	width = 0;
	height = 0;
}

bool ImageSource::init(int apiCode, System::String^ basePath, System::String^ filePath, System::String^ start, System::String^ length, double fps0, int interval)
{
	int lengthi = 0;

	reset();

	sourcePath->setInputPath(basePath, filePath);

	nfiles = sourcePath->getFileCount();
	if (nfiles == 0)
	{
		throw gcnew System::Exception("File(s) not found: " + sourcePath->templatePath);
	}

	this->start = Util::parseFrameTime(start, fps0);
	lengthi = Util::parseFrameTime(length, fps0);

	if (lengthi > 0)
	{
		this->end = this->start + lengthi;
		if (this->end > nfiles)
		{
			this->end = nfiles;
		}
	}

	if (this->end == 0)
	{
		this->end = nfiles;
	}

	this->interval = interval;
	if (this->interval == 0)
	{
		this->interval = 1;
	}

	sourcePath->resetFilePath();

	return open();
}

bool ImageSource::open()
{
	return true;
}

void ImageSource::close()
{
}

bool ImageSource::getNextImage(Mat* image)
{
	bool more = false;
	System::String^ filename = sourcePath->createFilePath(filei);
	
	label = Util::extractFileName(filename);

	if (filename != "")
	{
		*image = Util::loadImage(filename);
		if (Util::isValidImage(image))
		{
			width = image->cols;
			height = image->rows;
		}
		else
		{
			throw gcnew System::Exception("Image load error");
		}
		filei += interval;
		more = (filei < end);
	}

	if (!more)
	{
		close();
	}
	return more;
}

int ImageSource::getWidth()
{
	return width;
}

int ImageSource::getHeight()
{
	return height;
}

double ImageSource::getFps()
{
	return 0;
}

int ImageSource::getFrameNumber()
{
	return filei;
}

System::String^ ImageSource::getLabel()
{
	return label;
}

int ImageSource::getCurrentFrame()
{
	return filei - start;
}

int ImageSource::getTotalFrames()
{
	return end - start;
}
