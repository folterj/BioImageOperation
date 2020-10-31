/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageItemList.h"
#include "Util.h"


ImageItemList::ImageItemList() {
}

ImageItemList::~ImageItemList() {
	for (int i = 0; i < size(); i++) {
		delete at(i);
	}
}

void ImageItemList::reset() {
	clear();
}

Mat* ImageItemList::getImage(string label, bool mustExist) {
	if (label != "") {
		for (int i = 0; i < size(); i++) {
			if (at(i)->label == label) {
				return &at(i)->image;
			}
		}
	}
	if (mustExist) {
		throw invalid_argument("Image not found: " + label);
	}
	return NULL;
}

void ImageItemList::setImage(Mat* image, string label) {
	ImageItem* item = nullptr;

	if (label == "") {
		throw invalid_argument("Invalid label");
	}

	if (!Util::isValidImage(image)) {
		throw invalid_argument("Invalid image");
	}

	for (int i = 0; i < size(); i++) {
		if (at(i)->label == label) {
			item = at(i);
		}
	}
	if (!item) {
		item = new ImageItem(label);
		push_back(item);
	}
	item->image = *image;
}
