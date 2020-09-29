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

#include "VideoSource.h"
#include "Constants.h"
#include "Util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


VideoSource::VideoSource()
{
}

VideoSource::~VideoSource()
{
	close();
}

void VideoSource::reset()
{
	sourcePath->reset();
	apiCode = VideoCaptureAPIs::CAP_ANY;
	label = "";
	nsources = 0;
	sourcei = 0;
	nframes = 0;
	videoNframes = 0;
	framei = 0;
	videoFramei = 0;
	width = 0;
	height = 0;
	fps = 1;
	start = 0;
	end = 0;
	interval = 1;
	seekMode = false;
	close();
}

bool VideoSource::init(int apiCode, System::String^ basePath, System::String^ filePath, System::String^ start, System::String^ length, double fps0, int interval)
{
	System::String^ fileName = ".";	// dummy value to pass initial while-loop condition
	bool ok = false;
	bool canSeek;
	int lengthi = 0;
	int nframes0;
	System::String^ message;

	reset();
	this->apiCode = apiCode;
	sourcePath->setInputPath(basePath, filePath);

	nsources = sourcePath->getFileCount();
	if (nsources == 0)
	{
		throw gcnew System::Exception("File(s) not found: " + sourcePath->templatePath);
	}

	nframes = 0;
	while (fileName != "")
	{
		fileName = sourcePath->createFilePath();
		if (fileName != "")
		{
			if (videoCapture.open(Util::stdString(fileName)))
			{
				nframes0 = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);
				if (nframes0 > 0) {
					nframes += nframes0;
				}
				width = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
				height = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
				fps = videoCapture.get(VideoCaptureProperties::CAP_PROP_FPS);
				if (fps < 0) {
					fps = 0;
				}
			}
			else
			{
				message = "Unable to open capture";
				if (apiCode != 0) {
					message += " API code: " + apiCode;
				}
				message += " filename: " + fileName;
				throw gcnew System::Exception(message);

			}
			videoCapture.release();
		}
	}
	sourcePath->resetFilePath();

	if (fps == 0)
	{
		fps = fps0;
	}

	this->start = Util::parseFrameTime(start, fps);
	lengthi = Util::parseFrameTime(length, fps);

	if (lengthi > 0)
	{
		this->end = this->start + lengthi;
		if (this->end > nframes)
		{
			this->end = 0;
		}
	}

	this->interval = interval;
	if (this->interval == 0)
	{
		this->interval = 1;
	}
	canSeek = (nframes != 0);
	seekMode = (canSeek && interval >= Constants::seekModeInterval);		// auto select seek mode: if interval >= x frames

	ok = open();
	if (ok)
	{
		framei = this->start;
		videoFramei = this->start;
		if (canSeek)
		{
			seekFrame();
		}
		else
		{
			for (int i = 0; i < this->start; i++)
			{
				nextFrame();
			}
		}
	}

	return ok;
}

bool VideoSource::open()
{
	bool ok = videoIsOpen;
	System::String^ fileName;
	System::String^ message;

	if (!videoIsOpen)
	{
		// open (next) video
		fileName = sourcePath->createFilePath();
		if (fileName != "")
		{
			if (videoCapture.open(Util::stdString(fileName), apiCode))
			{
				videoNframes = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);
				if (videoNframes < 0)
				{
					videoNframes = 0;
				}
				label = Util::extractFileName(fileName);
				sourcei++;
				videoIsOpen = videoCapture.isOpened();
				ok = videoIsOpen;
			}

			if (!videoIsOpen)
			{
				close();
				message = "Unable to open capture";
				if (apiCode != 0) {
					message += " API code: " + apiCode;
				}
				message += " filename: " + fileName;
				throw gcnew System::Exception(message);
			}
		}
		else
		{
			ok = false;
		}
	}
	return ok;
}

void VideoSource::release()
{
	videoCapture.release();
	videoIsOpen = false;
}

void VideoSource::close()
{
	release();
}

bool VideoSource::getNextImage(Mat* image)
{
	bool frameOk = false;

	if (seekMode)
	{
		if (seekFrame())
		{
			frameOk = nextFrame();
		}
		videoFramei += interval;
		framei += interval;
	}
	else
	{
		// skip frames
		do
		{
			frameOk = nextFrame();
			if (!frameOk)
			{
				break;
			}
			framei++;
		} while ((framei % interval) != 0);
	}

	if (frameOk)
	{
		if (end != 0 && framei >= end)
		{
			// reached desired length
			videoIsOpen = false;
		}
		else if (!videoCapture.retrieve(*image))
		{
			// unexpected error
			videoIsOpen = false;
		}
	}

	return (frameOk && videoIsOpen);
}

bool VideoSource::seekFrame()
{
	bool openOk = true;

	while (videoNframes != 0 && videoFramei >= videoNframes && openOk)
	{
		videoFramei -= videoNframes;
		videoCapture.release();
		videoIsOpen = false;
		openOk = open();
	}
	return videoCapture.set(VideoCaptureProperties::CAP_PROP_POS_FRAMES, videoFramei);
}

bool VideoSource::nextFrame()
{
	bool frameOk = false;

	do
	{
		if (!videoIsOpen)
		{
			// try open (next) video
			if (!open())
			{
				break;
			}
		}
		if (videoIsOpen)
		{
			frameOk = videoCapture.grab();
			if (!frameOk)
			{
				release();
			}
		}
	} while (!videoIsOpen);

	return (frameOk && videoIsOpen);
}

int VideoSource::getWidth()
{
	return width;
}

int VideoSource::getHeight()
{
	return height;
}

double VideoSource::getFps()
{
	return fps;
}

int VideoSource::getFrameNumber()
{
	return framei;
}

System::String^ VideoSource::getLabel()
{
	return label;
}

int VideoSource::getCurrentFrame()
{
	return framei - start;
}

int VideoSource::getTotalFrames()
{
	if (end > 0)
	{
		return end - start;
	}
	return nframes - start;
}
