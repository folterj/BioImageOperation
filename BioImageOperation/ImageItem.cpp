/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageItem.h"
#include "Util.h"


ImageItem::ImageItem(Mat* image, string label) {
	this->image = *image;
	this->label = label;
}

ImageItem::ImageItem(string label) {
	this->label = label;
}
