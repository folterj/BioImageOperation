/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
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
	sourcePath.reset();
	nfiles = 0;
	filei = 0;
	start = 0;
	end = 0;
	interval = 1;
	width = 0;
	height = 0;
}

bool ImageSource::init(int apiCode, string basePath, string filePath, string start, string length, double fps0, int interval)
{
	int lengthi = 0;

	reset();

	sourcePath.setInputPath(basePath, filePath);

	nfiles = sourcePath.getFileCount();
	if (nfiles == 0)
	{
		throw ios_base::failure("File(s) not found: " + sourcePath.templatePath);
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

	sourcePath.resetFilePath();

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
	string filename = sourcePath.createFilePath(filei);
	
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
			throw ios_base::failure("Image load error " + filename);
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

string ImageSource::getLabel()
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
