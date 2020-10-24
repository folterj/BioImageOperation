/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageOutput.h"
#include "Util.h"


ImageOutput::ImageOutput()
{
}

ImageOutput::~ImageOutput()
{
	close();
}

void ImageOutput::reset()
{
	outputPath.reset();
	start = 0;
	end = 0;
	filei = 0;
}

void ImageOutput::init(string basepath, string filepath, string defaultExtension, string start, string length, double fps0, string codecs)
{
	reset();
	int lengthi;

	if (!outputPath.setOutputPath(basepath, filepath, defaultExtension))
	{
		throw ios_base::failure("Unable to write to " + Util::extractFilePath(outputPath.initialPath));
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
		Util::saveImage(outputPath.createFilePath(filei), image);
	}
	filei++;
	return true;
}

void ImageOutput::close()
{
}
