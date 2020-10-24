/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
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
	apiCode = VideoCaptureAPIs::CAP_ANY;
	framei = 0;
	width = 0;
	height = 0;
	interval = 1;
	close();
}

bool CaptureSource::init(int apiCode, string basepath, string filepath, string start, string length, double fps0, int interval)
{
	reset();

	this->apiCode = apiCode;
	this->source = filepath;

	return open();
}

bool CaptureSource::open()
{
	bool isSourceIndex = false;
	int index;
	string message;

	if (!videoIsOpen)
	{
		if (Util::isNumeric(source))
		{
			index = stoi(source);
			isSourceIndex = true;
		}

		if (isSourceIndex)
		{
			if (videoCapture.open(index, apiCode))
			{
				videoIsOpen = videoCapture.isOpened();
			}
		}
		else
		{
			if (videoCapture.open(source, apiCode))
			{
				videoIsOpen = videoCapture.isOpened();
			}
		}

		if (!videoIsOpen)
		{
			close();
			message = "Unable to open capture";
			if (apiCode != 0) {
				message += " API code: " + apiCode;
			}
			message += " source: " + source;
			throw ios_base::failure(message);
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

int CaptureSource::getFrameNumber()
{
	return framei;
}

string CaptureSource::getLabel()
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
