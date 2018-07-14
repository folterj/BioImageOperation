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

#pragma once

#include <vector>
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace cv;


// value / ref / pointer:
// https://stackoverflow.com/questions/334856/are-there-benefits-of-passing-by-pointer-over-passing-by-reference-in-c
// http://www.cplusplus.com/articles/z6vU7k9E/

// problem: returning (or storing) a pointer to an object that gets destroyed, like a local object defined inside a function

// OpenCV Mat:
// Mat B = A.clone();  // B is a deep copy of A. (has its own copy of the pixels)
// Mat C = A;          // C is a shallow copy of A ( rows, cols copied, but shared pixel-pointer )
// Mat D; A.copyTo(D); // D is a deep copy of A, like B

// Best way to create (and return) a Mat
// http://answers.opencv.org/question/96739/returning-a-mat-from-a-function/
// * using Mat* pointer is faster than copying (or using Mat&)!

/*
 * Labelled image for searchable list of images
 */

class ImageItem
{
public:
	Mat image;
	std::string label;

	ImageItem(System::String^ label);
	ImageItem(Mat* image, System::String^ label);
};
