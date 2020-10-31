/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <vector>
#include "ImageItem.h"

using namespace std;


/*
 * Searchable list of images
 */

class ImageItemList : std::vector<ImageItem*>
{
public:
	ImageItemList();
	~ImageItemList();

	void reset();
	Mat* getImage(string label, bool mustExist = true);
	void setImage(Mat* image, string label);
};
