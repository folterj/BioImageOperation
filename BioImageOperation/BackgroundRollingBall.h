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
#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

#include "Constants.h"

using namespace System;
using namespace cv;

/*
Fully Ported to Python from ImageJ's Background Subtractor.
Only works for 8-bit greyscale images currently.
Based on the concept of the rolling ball algorithm described
in Stanley Sternberg's article,
"Biomedical Image Processing", IEEE Computer, January 1983.
Imagine that the 2D grayscale image has a third (height) dimension by the image
value at every point in the image, creating a surface. A ball of given radius
is rolled over the bottom side of this surface; the hull of the volume
reachable by the ball is the background.
http://rsbweb.nih.gov/ij/developer/source/ij/plugin/filter/BackgroundSubtracter.java.html
*/


/*
Calculates and subtracts or creates background from image.

Parameters
----------
img : uint8 np array
	Image
radius : int
	Radius of the rolling ball creating the background (actually a
					paraboloid of rotation with the same curvature)
light_background : bool
	Whether the image has a light background.
do_presmooth : bool
	Whether the image should be smoothened (3x3 mean) before creating
					the background. With smoothing, the background will not necessarily
					be below the image data.
use_paraboloid : bool
	Whether to use the "sliding paraboloid" algorithm.

Returns
-------
img, background : uint8 np array
	Background subtracted image, Background
*/


enum Direction {
	x_direction,
	y_direction,
	diagonal_1a,
	diagonal_1b,
	diagonal_2a,
	diagonal_2b,
};


public class RollingBall {
	/*
		A rolling ball (or actually a square part thereof)
		Here it is also determined whether to shrink the image
	*/
public:
	double* data;
	int width = 0;
	double shrink_factor;

	RollingBall(int radius);
	void build(int ball_radius, int arc_trim_per);
};


public class BackgroundSubtract {
public:
	int width = 0;
	int height = 0;

	int s_width = 0;
	int s_height = 0;

	static Mat* subtract_background_rolling_ball(Mat* img, int radius, bool light_background, bool use_paraboloid = false, bool do_presmooth = true);

	BackgroundSubtract();

	/*
	Calculates and subtracts or creates background from image.

	Parameters
	----------
	img : uint8 np array
		Image
	radius : int
		Radius of the rolling ball creating the background (actually a
						paraboloid of rotation with the same curvature)
	light_background : bool
		Whether the image has a light background.
	do_presmooth : bool
		Whether the image should be smoothened (3x3 mean) before creating
						the background. With smoothing, the background will not necessarily
						be below the image data.
	use_paraboloid : bool
		Whether to use the "sliding paraboloid" algorithm.

	Returns
	-------
	img, (background) : uint8 np array
	Background subtracted image, (Background)

	*/
	Mat* rolling_ball_background(Mat* img, int radius, bool light_background, bool use_paraboloid, bool do_presmooth);

	/*
	Applies a 3x3 mean filter to specified array.
	*/
	Mat* _smooth(Mat* img, int window = 3);

	Mat* _rolling_ball_float_background(Mat* float_img, bool invert, RollingBall* ball);

	void _roll_ball(RollingBall* ball, Mat* float_img);

	Mat _shrink_image(Mat* img, double shrink_factor);

	Mat* _enlarge_image(Mat* small_img, Mat* float_img, double shrink_factor);

	int* _make_interpolation_indices(int length, int s_length, double shrink_factor);
	double* _make_interpolation_weights(int length, int s_length, double shrink_factor);

	Mat* _sliding_paraboloid_float_background(Mat* float_img, int radius, bool invert);

	void _correct_corners(Mat* float_img, double coeff2, double* cache, int* next_point);

	double* _line_slide_parabola(Mat* float_img, int startx, int starty, int incx, int incy, int length, double coeff2, double* cache, int* next_point, double* corrected_edges);

	void _filter1d(Mat* float_img, Direction direction, double coeff2, double* cache, int* next_point);
};
