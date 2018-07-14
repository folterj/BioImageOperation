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

#include "VideoOutput.h"
#include "Constants.h"
#include "Util.h"

using namespace System::IO;

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
	outputPath->reset();
	width = 0;
	height = 0;
	isColor = true;
	fps = 0;
	codec = 0;
	close();
}

void VideoOutput::init(System::String^ basePath, System::String^ filePath, System::String^ defaultExtension, System::String^ start, System::String^ length, double fps, System::String^ codecs)
{
	System::String^ filename;

	reset();

	outputPath->setOutputPath(basePath, filePath, defaultExtension);
	this->fps = fps;

	if (codecs == "")
	{
		codecs = Util::netString(Constants::defaultVideoCodec);
	}
	codecs = codecs->ToUpper();
	codec = VideoWriter::fourcc((char)codecs[0], (char)codecs[1], (char)codecs[2], (char)codecs[3]);
	
	filename = outputPath->createFilePath(0);
	if (File::Exists(filename))
	{
		throw gcnew IOException(System::String::Format("Output file {0} already exists", filename));
	}
}

bool VideoOutput::open()
{
	bool ok = videoIsOpen;
	System::String^ fileName;

	if (!videoIsOpen)
	{
		fileName = outputPath->createFilePath();
		if (fileName != "")
		{
			if (videoWriter.open(Util::stdString(fileName), codec, fps, cv::Size(width, height), isColor))
			{
				videoIsOpen = videoWriter.isOpened();
				ok = videoIsOpen;
			}

			if (!videoIsOpen)
			{
				close();
				throw gcnew System::Exception(System::String::Format("Unable to create video: {0} with encoding: {1} @ {2}x{3}@{4}fps", fileName, Util::getCodecString(codec), width, height, fps));
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
