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

#include "ImageTrackers.h"


ImageTrackers::ImageTrackers()
{
}

ImageTrackers::~ImageTrackers()
{
	for (int i = 0; i < size(); i++)
	{
		delete at(i);
	}
	clear();
}

void ImageTrackers::reset()
{
	for (int i = 0; i < size(); i++)
	{
		delete at(i);
	}
	clear();
}

void ImageTrackers::close()
{
	for (int i = 0; i < size(); i++)
	{
		at(i)->closeStreams();
	}
}

ImageTracker* ImageTrackers::getTracker(System::String^ trackerId, bool firstCreate)
{
	for (int i = 0; i < size(); i++)
	{
		if (at(i)->trackerId == trackerId)
		{
			return at(i);
		}
	}
	if (firstCreate)
	{
		ImageTracker* newTracker = new ImageTracker(trackerId);
		push_back(newTracker);
		return newTracker;
	}
	throw gcnew ArgumentOutOfRangeException(System::String::Format("Tracker with ID: {0} not found", trackerId));
	return NULL;
}
