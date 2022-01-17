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


class OpticalCorrection
{
public:
	Mat map1, map2;
	bool calibrated = false;

	bool calibrate(InputArray calibration_image, int calibrationx, int calibrationy);
	bool undistort(InputArray source, OutputArray dest);
	static vector<vector<Point3f>> calc_points_mesh(vector<Point2f> points, int calibrationx, int calibrationy);
};
