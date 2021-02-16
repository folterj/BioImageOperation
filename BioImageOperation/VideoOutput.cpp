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


VideoOutput::VideoOutput() {
}

VideoOutput::~VideoOutput() {
	close();
}

void VideoOutput::reset() {
	outputPath.reset();
	width = 0;
	height = 0;
	isColor = true;
	fps = 0;
	codec = 0;
	close();
}

void VideoOutput::init(string basepath, string filepath, string defaultExtension, string start, string length, double fps, string codecs) {
	reset();

	outputPath.setOutputPath(basepath, filepath, defaultExtension);
	this->fps = fps;

	if (codecs == "") {
		codecs = Constants::defaultVideoCodec;
	}
	Util::toUpper(codecs);
	codec = VideoWriter::fourcc((char)codecs[0], (char)codecs[1], (char)codecs[2], (char)codecs[3]);
}

bool VideoOutput::open() {
	bool ok = videoIsOpen;
	string filename;

	if (!videoIsOpen) {
		filename = outputPath.createFilePath();
		if (filename != "") {
			if (videoWriter.open(filename, codec, fps, cv::Size(width, height), isColor)) {
				videoIsOpen = videoWriter.isOpened();
				ok = videoIsOpen;
			}

			if (!videoIsOpen) {
				close();
				throw invalid_argument(Util::format("Unable to create video: %s with encoding: %s @ %dx%d@%ffps", filename.c_str(), Util::getCodecString(codec).c_str(), width, height, fps));
			}
		} else {
			ok = false;
		}
	}
	return ok;
}

bool VideoOutput::writeImage(Mat* image) {
	if (Util::isValidImage(image)) {
		if (!videoIsOpen) {
			width = image->cols;
			height = image->rows;
			isColor = (image->channels() > 1);
			open();
		}

		if (videoIsOpen) {
			videoWriter.write(*image);
			return true;
		}
	}
	return false;
}

void VideoOutput::close() {
	videoWriter.release();
	videoIsOpen = false;
}
