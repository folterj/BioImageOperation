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

#include "ImageItemList.h"
#include "Util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


ImageItemList::ImageItemList()
{
}

ImageItemList::~ImageItemList()
{
	for (int i = 0; i < size(); i++)
	{
		delete at(i);
	}
}

void ImageItemList::reset()
{
	clear();
}

Mat* ImageItemList::getImage(System::String^ label, bool mustExist)
{
	if (label != "")
	{
		for (int i = 0; i < size(); i++)
		{
			if (Util::netString(at(i)->label) == label)
			{
				return &at(i)->image;
			}
		}
	}
	if (mustExist)
	{
		throw gcnew System::Exception("Image not found: " + label);
	}
	return NULL;
}

void ImageItemList::setImage(Mat* image, System::String^ label)
{
	ImageItem* item = nullptr;

	if (label == "")
	{
		throw gcnew System::Exception("Invalid label");
	}

	if (!Util::isValidImage(image))
	{
		throw gcnew System::Exception("Invalid image");
	}

	for (int i = 0; i < size(); i++)
	{
		if (Util::netString(at(i)->label) == label)
		{
			item = at(i);
		}
	}
	if (!item)
	{
		item = new ImageItem(label);
		push_back(item);
	}
	item->image = *image;
}
