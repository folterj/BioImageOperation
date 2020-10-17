/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include "FrameOutput.h"
#include "NumericPath.h"


/*
 * Output for image file(s)
 */

class ImageOutput : public FrameOutput
{
public:
	NumericPath outputPath;
	int start;
	int end;
	int filei = 0;

	ImageOutput();
	~ImageOutput();
	void reset();
	void init(string basePath, string filePath, string defaultExtension = "", string start = "", string length = "", double fps0 = 1, string codecs = "");
	bool writeImage(Mat* image);
	void close();
};
