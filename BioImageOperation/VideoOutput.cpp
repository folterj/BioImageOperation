/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <filesystem>
#include "VideoOutput.h"
#include "Constants.h"
#include "Util.h"


// Regarding ffmpeg: https://github.com/opencv/opencv/tree/master/3rdparty/ffmpeg


VideoOutput::VideoOutput()
{
}

VideoOutput::~VideoOutput()
{
	close();
}

void VideoOutput::reset()
{
	outputPath.reset();
	width = 0;
	height = 0;
	isColor = true;
	fps = 0;
	codec = 0;
	close();
}

void VideoOutput::init(string basePath, string filePath, string defaultExtension, string start, string length, double fps, string codecs)
{
	string filename;

	reset();

	outputPath.setOutputPath(basePath, filePath, defaultExtension);
	this->fps = fps;

	if (codecs == "")
	{
		codecs = Constants::defaultVideoCodec;
	}
	Util::toUpper(codecs);
	codec = VideoWriter::fourcc((char)codecs[0], (char)codecs[1], (char)codecs[2], (char)codecs[3]);
	
	filename = outputPath.createFilePath(0);
	if (filesystem::exists(filename))
	{
		throw ios_base::failure("Output file already exists " + filename);
	}
}

bool VideoOutput::open()
{
	bool ok = videoIsOpen;
	string filename;

	if (!videoIsOpen)
	{
		filename = outputPath.createFilePath();
		if (filename != "")
		{
			if (videoWriter.open(filename, codec, fps, cv::Size(width, height), isColor))
			{
				videoIsOpen = videoWriter.isOpened();
				ok = videoIsOpen;
			}

			if (!videoIsOpen)
			{
				close();
				throw invalid_argument("Unable to create video: " + filename +
										" with encoding: " + Util::getCodecString(codec) +
										" @ " + to_string(width) +
										"x" + to_string(height) +
										"@" + to_string(fps) + "fps");
			}
		}
		else
		{
			ok = false;
		}
	}
	return ok;
}

bool VideoOutput::writeImage(Mat* image)
{
	if (Util::isValidImage(image))
	{
		if (!videoIsOpen)
		{
			width = image->cols;
			height = image->rows;
			isColor = (image->channels() > 1);
			open();
		}

		if (videoIsOpen)
		{
			videoWriter.write(*image);
			return true;
		}
	}
	return false;
}

void VideoOutput::close()
{
	videoWriter.release();
	videoIsOpen = false;
}
