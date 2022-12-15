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


CaptureSource::CaptureSource() {
}

CaptureSource::~CaptureSource() {
	close();
}

void CaptureSource::reset() {
	source = "";
	apiCode = VideoCaptureAPIs::CAP_ANY;
	framei = 0;
	width = 0;
	height = 0;
	interval = 1;
	close();
}

bool CaptureSource::init(string basepath, string filepath, int apiCode, string codecs, string start, string length,
						 double fps, int interval, int total, int width, int height) {
	int codec;
	int width0, height0;
	double fps0;

	reset();

	this->apiCode = apiCode;
	this->source = filepath;

	capParams.clear();
	if (width > 0) {
		capParams.push_back(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
		capParams.push_back(width);
	}
	if (height > 0) {
		capParams.push_back(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
		capParams.push_back(height);
	}
	if (fps > 0) {
		capParams.push_back(VideoCaptureProperties::CAP_PROP_FPS);
		capParams.push_back(fps);
	}
	// * Set codec AFTER width/height
	if (codecs != "") {
		Util::toUpper(codecs);
		codec = VideoWriter::fourcc((char)codecs[0], (char)codecs[1], (char)codecs[2], (char)codecs[3]);
		capParams.push_back(VideoCaptureProperties::CAP_PROP_FOURCC);
		capParams.push_back(codec);
	}

	if (open()) {
		width0 = videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
		if (width0 > 0) {
			width = width0;
		}
		height0 = videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
		if (height0 > 0) {
			height = height0;
		}
		fps0 = videoCapture.get(VideoCaptureProperties::CAP_PROP_FPS);
		if (fps0 > 0) {
			fps = fps0;
		}
		calcFrameParams(start, length, fps, interval, total, 0);
		return true;
	}
	return false;
}

bool CaptureSource::open() {
	bool isSourceIndex = false;
	int index;
	string message;

	if (!videoIsOpen) {
		if (Util::isNumeric(source)) {
			index = stoi(source);
			isSourceIndex = true;
		}

		if (isSourceIndex) {
			if (videoCapture.open(index, apiCode, capParams)) {
				videoIsOpen = videoCapture.isOpened();
			}
		} else {
			if (videoCapture.open(source, apiCode, capParams)) {
				videoIsOpen = videoCapture.isOpened();
			}
		}

		if (!videoIsOpen) {
			close();
			message = "Unable to open capture";
			if (apiCode != 0) {
				message += " API code: " + apiCode;
			}
			message += " source: " + source;
			throw ios_base::failure(message);
		} else {
			width = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
			height = (int)videoCapture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
			fps = videoCapture.get(VideoCaptureProperties::CAP_PROP_FPS);
		}
	}
	return videoIsOpen;
}

bool CaptureSource::getNextImage(Mat* image) {
	bool frameOk = false;

	do {
		frameOk = videoCapture.read(*image);	// need blocking call to respect interval
		if (!frameOk) {
			close();
			break;
		}
		framei++;
	} while ((framei % interval) != 0);

	if (frameOk) {
		if (end > 0 && framei >= end) {
			// reached desired length
			videoIsOpen = false;
		}
	}

	return (frameOk && videoIsOpen);
}

void CaptureSource::close() {
	videoCapture.release();
	videoIsOpen = false;
}

int CaptureSource::getWidth() {
	return width;
}

int CaptureSource::getHeight() {
	return height;
}

double CaptureSource::getFps() {
	return fps;
}

int CaptureSource::getFrameNumber() {
	return framei;
}

string CaptureSource::getLabel() {
	return "";
}

int CaptureSource::getCurrentFrame() {
	return framei;
}

int CaptureSource::getTotalFrames() {
	return 0;
}
