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

#include "CaptureSource.h"
#include "Util.h"


CaptureSource::CaptureSource()
{
}

CaptureSource::~CaptureSource()
{
	close();
}

void CaptureSource::reset()
{
	source = "";
	framei = 0;
	width = 0;
	height = 0;
	interval = 1;
	close();
}

bool CaptureSource::init(System::String^ basePath, System::String^ filePath, System::String^ start, System::String^ length, double fps0, int interval)
{
	reset();

	this->source = filePath;

	return open();
}

bool CaptureSource::open()
{
	bool isSourceIndex = false;
	int index;

	if (!videoIsOpen)
	{
		if (Util::isNumeric(source))
		{
			index = int::Parse(source);
			isSourceIndex = true;
		}

		if (isSourceIndex)
		{
			if (videoCapture.open(index))
			{
				videoIsOpen = videoCapture.isOpened();
			}
		}
		else
		{
			if (videoCapture.open(Util::stdString(source)))
			{
				videoIsOpen = videoCapture.isOpened();
			}
		}

		if (!videoIsOpen)
		{
			close();
			throw gcnew System::Exception("Unable to open capture source: " + source);
		}
		else
		{
			width = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
			height = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
			fps = videoCapture.get(VideoCaptureProperties::CAP_PROP_FPS);
		}
	}
	return videoIsOpen;
}

bool CaptureSource::getNextImage(Mat* image)
{
	bool frameOk = false;

	do
	{
		frameOk = videoCapture.grab();
		if (!frameOk)
		{
			close();
			break;
		}
		framei++;
	} while ((framei % interval) != 0);

	if (frameOk)
	{
		if (!videoCapture.retrieve(*image))
		{
			videoIsOpen = false;
		}
	}

	return (frameOk && videoIsOpen);
}

void CaptureSource::close()
{
	videoCapture.release();
	videoIsOpen = false;
}

int CaptureSource::getWidth()
{
	return width;
}

int CaptureSource::getHeight()
{
	return height;
}

double CaptureSource::getFps()
{
	return fps;
}

System::String^ CaptureSource::getLabel()
{
	return "";
}

int CaptureSource::getCurrentFrame()
{
	return framei;
}

int CaptureSource::getTotalFrames()
{
	return 0;
}
