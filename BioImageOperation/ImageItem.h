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
#include <opencv2/opencv.hpp>

using namespace std;
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
	string label;

	ImageItem(string label);
	ImageItem(Mat* image, string label);
};
