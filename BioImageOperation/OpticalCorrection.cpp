/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "OpticalCorrection.h"
#include "ImageOperations.h"


bool OpticalCorrection::calibrate(InputArray calibration_image, int calibrationx, int calibrationy) {
	Mat calibration_image2;
	Size calibration_size = Size(calibrationx, calibrationy);
	Size size = calibration_image.size();
	Size windowSize = Size((int)(size.width / calibrationx / 10), (int)(size.height / calibrationy / 10));
	Mat cameraMatrix, newCameraMatrix, distCoeffs;
	vector<Mat> rvecs, tvecs;
	vector<Point2f> points;
	vector<vector<Point2f>> points2;
	vector<vector<Point3f>> mesh3d;
	int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FILTER_QUADS;
	bool found;

	ImageOperations::convertToGrayScale(calibration_image, calibration_image2);
	found = findChessboardCorners(calibration_image2, calibration_size, points, chessBoardFlags);
	if (found) {
		cornerSubPix(calibration_image2, points, windowSize, Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
		points2.push_back(points);
		mesh3d = calc_points_mesh(points, calibrationx, calibrationy);
		calibrateCamera(mesh3d, points2, size, cameraMatrix, distCoeffs, rvecs, tvecs);
		newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, size, 0);
		initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), newCameraMatrix, size, CV_32FC1, map1, map2);
	}
	calibrated = found;
	return found;
}

bool OpticalCorrection::undistort(InputArray source, OutputArray dest) {
	if (calibrated) {
		remap(source, dest, map1, map2, cv::INTER_LINEAR);
		return true;
	}
	return false;
}

vector<vector<Point3f>> OpticalCorrection::calc_points_mesh(vector<Point2f> points, int calibrationx, int calibrationy) {
	vector<vector<Point3f>> mesh3d{{}};
	double xmin, xmax, ymin, ymax, sum_min, sum_max;
	double x, y;

	sum_min = 0;
	sum_max = 0;
	for (int i = 0; i < points.size(); i += calibrationx) {
		sum_min += points[i].x;
		sum_max += points[i + (calibrationx - 1)].x;
	}
	xmin = sum_min / calibrationy;
	xmax = sum_max / calibrationy;

	sum_min = 0;
	sum_max = 0;
	for (int i = 0; i < calibrationx; i++) {
		sum_min += points[i].y;
		sum_max += points[points.size() - 1 - i].y;
	}
	ymin = sum_min / calibrationx;
	ymax = sum_max / calibrationx;

	for (int yi = 0; yi < calibrationy; yi++) {
		y = ymin + (ymax - ymin) * yi / (calibrationy - 1);
		for (int xi = 0; xi < calibrationx; xi++) {
			x = xmin + (xmax - xmin) * xi / (calibrationx - 1);
			mesh3d[0].push_back(Point3f(x, y, 0));
		}
	}
	return mesh3d;
}
