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

#include "BackgroundRollingBall.h"

Mat* BackgroundSubtract::subtract_background_rolling_ball(Mat* img, int radius, bool light_background, bool use_paraboloid, bool do_presmooth) {
	BackgroundSubtract bs;
	return bs.rolling_ball_background(img, radius, light_background, use_paraboloid, do_presmooth);
}


RollingBall::RollingBall(int radius) {
	int arc_trim_per;

	if (radius <= 10) {
		shrink_factor = 1;
		arc_trim_per = 24;
	}
	else if (radius <= 30) {
		shrink_factor = 2;
		arc_trim_per = 24;
	}
	else if (radius <= 100) {
		shrink_factor = 4;
		arc_trim_per = 32;
	}
	else {
		shrink_factor = 8;
		arc_trim_per = 40;
	}
	build(radius, arc_trim_per);
}

void RollingBall::build(int ball_radius, int arc_trim_per) {
	int small_ball_radius = (int)(ball_radius / shrink_factor);
	int x_val, y_val, temp;

	if (small_ball_radius < 1) {
		small_ball_radius = 1;
	}

	int r_square = small_ball_radius * small_ball_radius;
	int x_trim = int(arc_trim_per * small_ball_radius / 100);
	int half_width = (int)Math::Round(small_ball_radius - x_trim);
	width = 2 * half_width + 1;
	data = new double[width * width]();

	int p = 0;
	for (int y = 0; y < width; y++) {
		for (int x = 0; x < width; x++) {
			x_val = x - half_width;
			y_val = y - half_width;

			temp = r_square - x_val * x_val - y_val * y_val;
			data[p] = (temp > 0) ? Math::Sqrt(temp) : 0;

			p++;
		}
	}
}


BackgroundSubtract::BackgroundSubtract() {
}

Mat* BackgroundSubtract::rolling_ball_background(Mat* img, int radius, bool light_background, bool use_paraboloid, bool do_presmooth) {
	RollingBall* ball;
	Mat float_img;
	bool invert;
	double offset;
	int value;

	height = img->rows;
	width = img->cols;
	s_height = height;
	s_width = width;

	if (img->channels() > 1) {
		cvtColor(*img, *img, COLOR_RGB2GRAY);
	}
	Mat _img = img->clone();
	if (do_presmooth) {
		_img = *_smooth(&_img);
	}

	//_img = _img.reshape(height * width);

	invert = false;
	if (light_background) {
		invert = true;
	}

	if (!use_paraboloid) {
		ball = new RollingBall(radius);
	}

	_img.convertTo(float_img, CV_64F);
	if (use_paraboloid) {
		float_img = *_sliding_paraboloid_float_background(&float_img, radius, invert);
	}
	else {
		float_img = *_rolling_ball_float_background(&float_img, invert, ball);
	}

	//float_img.convertTo(background, CV_8U);
	//background = background.reshape(height, width);

	offset = (invert) ? 255.5 : 0.5;
	for (int p = 0; p < width * height; p++) {
		value = (_img.at<uchar>(p) & 255) - float_img.at<double>(p) + offset;
		value = Math::Max(value, 0);
		value = Math::Min(value, 255);
		img->at<uchar>(int(p / width), int(p % width)) = value;
	}
	return img;		// , background;
}

Mat* BackgroundSubtract::_smooth(Mat* img, int window) {
	Mat kernel = Mat::ones(window, window, CV_64F);
	filter2D(*img, *img, -1, kernel);
	return img;
}

Mat* BackgroundSubtract::_rolling_ball_float_background(Mat* float_img, bool invert, RollingBall* ball) {
	Mat small_img;
	bool shrink = (ball->shrink_factor > 1);

	if (invert) {
		*float_img = Scalar(255) - *float_img;
	}

	//small_img = (shrink) ? _shrink_image(float_img, ball->shrink_factor) : *float_img;
	if (shrink) {
		s_height = int(height / ball->shrink_factor);
		s_width = int(width / ball->shrink_factor);
		resize(*float_img, small_img, cv::Size(s_width, s_height));
	} else {
		small_img = *float_img;
	}

	_roll_ball(ball, &small_img);

	if (shrink) {
		//float_img = _enlarge_image(&small_img, float_img, ball->shrink_factor);
		resize(small_img, *float_img, cv::Size(width, height));
	}

	if (invert) {
		*float_img = Scalar(255) - *float_img;
	}
	return float_img;
}

void BackgroundSubtract::_roll_ball(RollingBall* ball, Mat* float_img) {
	int height = s_height;
	int width = s_width;
	double* z_ball = ball->data;
	int ball_width = ball->width;
	int radius = int(ball_width / 2);
	int next_line_to_write, next_line_to_read;
	Mat* cache = new Mat(ball_width, width, CV_64F);		// double[width * ball_width]();
	double z, z_reduced, z_min;

	for (int y = -radius; y < height + radius; y++) {
		next_line_to_write = (y + radius) % ball_width;
		next_line_to_read = y + radius;
		if (next_line_to_read < height) {
			float_img->row(next_line_to_read).copyTo(cache->row(next_line_to_write));
			float_img->row(next_line_to_read).setTo(Scalar(double::NegativeInfinity));
		}

		int y0 = Math::Max(0, y - radius);
		int y_ball0 = y0 - y + radius;
		int y_end = y + radius;
		if (y_end >= height) {
			y_end = height - 1;
		}
		for (int x = -radius; x < width + radius; x++) {
			z = double::PositiveInfinity;
			int x0 = Math::Max(0, x - radius);
			int x_ball0 = x0 - x + radius;
			int x_end = x + radius;
			if (x_end >= width) {
				x_end = width - 1;
			}

			int y_ball = y_ball0;
			for (int yp = y0; yp < y_end + 1; yp++) {
				int cache_pointer = (yp % ball_width) * width + x0;
				int bp = x_ball0 + y_ball * ball_width;
				for (int xp = x0; xp < x_end + 1; xp++) {
					z_reduced = cache->at<double>(cache_pointer) - z_ball[bp];
					if (z > z_reduced) {
						z = z_reduced;
					}
					cache_pointer++;
					bp++;
				}
				y_ball++;
			}

			y_ball = y_ball0;
			for (int yp = y0; yp < y_end + 1; yp++) {
				int p = x0 + yp * width;
				int bp = x_ball0 + y_ball * ball_width;
				for (int xp = x0; xp < x_end + 1; xp++) {
					z_min = z + z_ball[bp];
					if (float_img->at<double>(p) < z_min) {
						float_img->at<double>(p) = z_min;
					}
					p++;
					bp++;
				}
				y_ball++;
			}
		}
	}
}

Mat BackgroundSubtract::_shrink_image(Mat* img, double shrink_factor) {
	// * use opencv resize call?
	s_height = int(height / shrink_factor);
	s_width = int(width / shrink_factor);

	//Mat img_copy = img->reshape(height, width).clone();
	Mat img_copy = img->clone();
	Mat small_img = Mat::ones(s_height, s_width, CV_64F);
	double min_value, max_value;
	for (int y = 0; y < s_height; y++) {
		for (int x = 0; x < s_width; x++) {
			double x_mask_min = shrink_factor * x;
			double y_mask_min = shrink_factor * y;
			minMaxLoc(img_copy(cv::Range(y_mask_min, y_mask_min + shrink_factor), cv::Range(x_mask_min, x_mask_min + shrink_factor)), &min_value, &max_value);
			small_img.at<double>(y, x) = min_value;
		}
	}
	//return small_img.reshape(s_height * s_width);
	return small_img;
}

Mat* BackgroundSubtract::_enlarge_image(Mat* small_img, Mat* float_img, double shrink_factor) {
	// * use opencv resize call?
	int* x_s_indices = _make_interpolation_indices(width, s_width, shrink_factor);
	double* x_weigths = _make_interpolation_weights(width, s_width, shrink_factor);
	int* y_s_indices = _make_interpolation_indices(height, s_height, shrink_factor);
	double* y_weights = _make_interpolation_weights(height, s_height, shrink_factor);
	double* line0 = new double[width]();
	double* line1 = new double[width]();
	for (int x = 0; x < width; x++) {
		line1[x] = small_img->at<double>(x_s_indices[x]) * x_weigths[x] + small_img->at<double>(x_s_indices[x] + 1) * (1.0 - x_weigths[x]);
	}
	int y_s_line0 = -1;
	for (int y = 0; y < height; y++) {
		if (y_s_line0 < y_s_indices[y]) {
			swap(line0, line1);
			y_s_line0 += 1;
			int s_y_ptr = int((y_s_indices[y] + 1) * s_width);
			for (int x = 0; x < width; x++) {
				line1[x] = small_img->at<double>(s_y_ptr + x_s_indices[x]) * x_weigths[x] + small_img->at<double>(s_y_ptr + x_s_indices[x] + 1) * (1.0 - x_weigths[x]);
			}
		}
		double weight = y_weights[y];
		int p = y * width;
		for (int x = 0; x < width; x++) {
			float_img->at<double>(p) = line0[x] * weight + line1[x] * (1.0 - weight);
			p++;
		}
	}
	return float_img;
}

int* BackgroundSubtract::_make_interpolation_indices(int length, int s_length, double shrink_factor) {
	int* s_indices = new int[length]();
	int s_idx;

	for (int i = 0; i < length; i++) {
		s_idx = int((i - shrink_factor / 2) / shrink_factor);
		if (s_idx >= s_length - 1) {
			s_idx = s_length - 2;
		}
		s_indices[i] = s_idx;
	}
	return s_indices;
}

double* BackgroundSubtract::_make_interpolation_weights(int length, int s_length, double shrink_factor) {
	double* weights = new double[length]();
	int s_idx;
	double distance;

	for (int i = 0; i < length; i++) {
		s_idx = int((i - shrink_factor / 2) / shrink_factor);
		if (s_idx >= s_length - 1) {
			s_idx = s_length - 2;
		}
		distance = (i + 0.5) / shrink_factor - (s_idx + 0.5);
		weights[i] = 1.0 - distance;
	}
	return weights;
}

Mat* BackgroundSubtract::_sliding_paraboloid_float_background(Mat* float_img, int radius, bool invert) {
	height, width = height, width;
	double* cache = new double[Math::Max(height, width)]();
	int* next_point = new int[Math::Max(height, width)]();
	double coeff2 = 0.5 / radius;
	double coeff2_diag = 1.0 / radius;

	if (invert) {
		*float_img = 255 - *float_img;
	}

	_correct_corners(float_img, coeff2, cache, next_point);
	_filter1d(float_img, Direction::x_direction, coeff2, cache, next_point);
	_filter1d(float_img, Direction::y_direction, coeff2, cache, next_point);
	_filter1d(float_img, Direction::x_direction, coeff2, cache, next_point);
	_filter1d(float_img, Direction::diagonal_1a, coeff2_diag, cache, next_point);
	_filter1d(float_img, Direction::diagonal_1b, coeff2_diag, cache, next_point);
	_filter1d(float_img, Direction::diagonal_2a, coeff2_diag, cache, next_point);
	_filter1d(float_img, Direction::diagonal_2b, coeff2_diag, cache, next_point);
	_filter1d(float_img, Direction::diagonal_1a, coeff2_diag, cache, next_point);
	_filter1d(float_img, Direction::diagonal_1b, coeff2_diag, cache, next_point);

	if (invert) {
		*float_img = 255 - *float_img;
	}

	return float_img;
}

void BackgroundSubtract::_correct_corners(Mat* float_img, double coeff2, double* cache, int* next_point) {
	double corners[4] = { };
	double* corrected_edges = new double[2]();
	corrected_edges = _line_slide_parabola(float_img, 0, 0, 1, 0, width, coeff2, cache, next_point, corrected_edges);
	corners[0] = corrected_edges[0];
	corners[1] = corrected_edges[1];
	corrected_edges = _line_slide_parabola(float_img, 0, height - 1, 1, 0, width, coeff2, cache, next_point, corrected_edges);
	corners[2] = corrected_edges[0];
	corners[3] = corrected_edges[1];
	corrected_edges = _line_slide_parabola(float_img, 0, 0, 0, 1, height, coeff2, cache, next_point, corrected_edges);
	corners[0] += corrected_edges[0];
	corners[2] += corrected_edges[1];
	corrected_edges = _line_slide_parabola(float_img, width - 1, 0, 0, 1, height, coeff2, cache, next_point, corrected_edges);
	corners[1] += corrected_edges[0];
	corners[3] += corrected_edges[1];
	double diag_length = Math::Min(width, height);
	double coeff2_diag = 2 * coeff2;
	corrected_edges = _line_slide_parabola(float_img, 0, 0, 1, 1, diag_length, coeff2_diag, cache, next_point, corrected_edges);
	corners[0] += corrected_edges[0];
	corrected_edges = _line_slide_parabola(float_img, width - 1, 0, -1, 1, diag_length, coeff2_diag, cache, next_point, corrected_edges);
	corners[1] += corrected_edges[0];
	corrected_edges = _line_slide_parabola(float_img, 0, height - 1, 1, -1, diag_length, coeff2_diag, cache, next_point, corrected_edges);
	corners[2] += corrected_edges[0];
	corrected_edges = _line_slide_parabola(float_img, width - 1, height - 1, -1, -1, diag_length, coeff2_diag, cache, next_point, corrected_edges);
	corners[3] += corrected_edges[0];

	float_img->at<double>(0, 0) = Math::Min(float_img->at<double>(0, 0), corners[0] / 3);
	float_img->at<double>(0, width - 1) = Math::Min(float_img->at<double>(0, width - 1), corners[1] / 3);
	float_img->at<double>(height - 1, 0) = Math::Min(float_img->at<double>(height - 1, 0), corners[2] / 3);
	float_img->at<double>(height - 1, width - 1) = Math::Min(float_img->at<double>(height - 1, width - 1), corners[3] / 3);
}

double* BackgroundSubtract::_line_slide_parabola(Mat* float_img, int startx, int starty, int incx, int incy, int length, double coeff2, double* cache, int* next_point, double* corrected_edges) {
	double min_value = double::PositiveInfinity;
	int last_point = 0;
	int first_corner = length - 1;
	int last_corner = 0;
	double v_prev1 = 0;
	double v_prev2 = 0;
	double curvature_test = 1.999 * coeff2;

	Point p(startx, starty);
	for (int i = 0; i < length; i++) {
		double v = float_img->at<double>(p);
		cache[i] = v;
		min_value = Math::Min(min_value, v);
		if (i >= 2 && v_prev1 + v_prev1 - v_prev2 - v < curvature_test) {
			next_point[last_point] = i - 1;
			last_point = i - 1;
		}
		v_prev2 = v_prev1;
		v_prev1 = v;

		p.x += incx;
		p.y += incy;
	}

	next_point[last_point] = length - 1;
	next_point[length - 1] = int::MaxValue;

	int i1 = 0;
	while (i1 < length - 1) {
		double v1 = cache[i1];
		double min_slope = double::PositiveInfinity;
		int i2 = 0;
		int search_to = length;
		int recalculate_limit_now = 0;

		int j = next_point[i1];
		while (j < search_to) {
			double v2 = cache[j];
			double slope = (v2 - v1) / (j - i1) + coeff2 * (j - i1);
			if (slope < min_slope) {
				min_slope = slope;
				i2 = j;
				recalculate_limit_now = -3;
			}
			if (recalculate_limit_now == 0) {
				double b = 0.5 * min_slope / coeff2;
				int max_search = i1 + int(b + Math::Sqrt(b * b + (v1 - min_value) / coeff2) + 1);
				if (0 < max_search && max_search < search_to) {
					search_to = max_search;
				}
			}

			j = next_point[j];
			recalculate_limit_now += 1;
		}

		if (i1 == 0) {
			first_corner = i2;
		}
		if (i2 == length - 1) {
			last_corner = i1;
		}
		p.x = startx + (i1 + 1) * incx;
		p.y = starty + (i1 + 1) * incy;
		for (j = i1 + 1; j < i2; j++) {
			float_img->at<double>(p) = v1 + (j - i1) * (min_slope - (j - i1) * coeff2);

			p.x += incx;
			p.y += incy;
		}
		i1 = i2;
	}
	if (corrected_edges) {
		if (4 * first_corner >= length) {
			first_corner = 0;
		}
		if (4 * (length - 1 - last_corner) >= length) {
			last_corner = length - 1;
		}
		double v1 = cache[first_corner];
		double v2 = cache[last_corner];
		double slope = (v2 - v1) / (last_corner - first_corner);
		double value0 = v1 - slope * first_corner;
		double coeff6 = 0;
		double mid = 0.5 * (last_corner + first_corner);
		for (int i = (length + 2) / 3; i < 2 * length / 3 + 1; i++) {
			double dx = (i - mid) * 2 / (last_corner - first_corner);
			double poly6 = dx * dx * dx * dx * dx * dx - 1;
			if (cache[i] < value0 + slope * i + coeff6 * poly6) {
				coeff6 = -(value0 + slope * i - cache[i]) / poly6;
			}
		}
		double dx = (first_corner - mid) * 2.0 / (last_corner - first_corner);
		corrected_edges[0] = value0 + coeff6 * (dx * dx * dx * dx * dx * dx - 1.0) + coeff2 * first_corner * first_corner;
		dx = (last_corner - mid) * 2.0 / (last_corner - first_corner);
		corrected_edges[1] = value0 + (length - 1) * slope + coeff6 * (dx * dx * dx * dx * dx * dx - 1.0) + coeff2 * (length - 1 - last_corner) * (length - 1 - last_corner);
	}
	return corrected_edges;
}

void BackgroundSubtract::_filter1d(Mat* float_img, Direction direction, double coeff2, double* cache, int* next_point) {
	int start = 0;
	int incx = 0;
	int incy = 0;
	int n = 0;
	int point_incx = 0;
	int point_incy = 0;
	int start_pixelx = 0;
	int start_pixely = 0;
	int length = 0;

	if (direction == Direction::x_direction) {
		n = height;
		incy = 1;
		point_incx = 1;
		length = width;
	} else if (direction == Direction::y_direction) {
		n = width;
		incx = 1;
		point_incy = 1;
		length = height;
	} else if (direction == Direction::diagonal_1a) {
		n = width - 2;
		incx = 1;
		point_incx = 1;
		point_incy = 1;
	} else if (direction == Direction::diagonal_1b) {
		start = 1;
		n = height - 2;
		incy = 1;
		point_incx = 1;
		point_incy = 1;
	} else if (direction == Direction::diagonal_2a) {
		start = 2;
		n = width;
		incx = 1;
		point_incx = -1;
		point_incy = 1;
	} else if (direction == Direction::diagonal_2b) {
		start = 0;
		n = height - 2;
		incy = 1;
		point_incx = -1;
		point_incy = 1;
	}
	for (int i = start; i < n; i++) {
		start_pixelx = i * incx;
		start_pixely = i * incy;
		if (direction == Direction::diagonal_1a) {
			length = Math::Min(height, width - i);
		} else if (direction == Direction::diagonal_1b) {
			length = Math::Min(width, height - i);
		} else if (direction == Direction::diagonal_2a) {
			length = Math::Min(height, i + 1);
		} else if (direction == Direction::diagonal_2b) {
			start_pixelx += width - 1;
			length = Math::Min(width, height - i);
		}
		_line_slide_parabola(float_img, start_pixelx, start_pixely, point_incx, point_incy, length, coeff2, cache, next_point, nullptr);
	}
}
