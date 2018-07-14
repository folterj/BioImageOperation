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

#include "AverageBuffer.h"


AverageBuffer::AverageBuffer()
{
}

void AverageBuffer::reset()
{
	set = false;
}

void AverageBuffer::addImage(Mat* image, double weight)
{
	if (!set)
	{
		// initialise background using current image
		image->convertTo(averageImage, CV_32F);
		set = true;
	}
	else
	{
		// add using weighting
		if (weight == 0)
		{
			weight = 0.01;
		}
		accumulateWeighted(*image, averageImage, weight);
	}
}

void AverageBuffer::getImage(Mat* dest)
{
	averageImage.convertTo(*dest, CV_8U);
}
