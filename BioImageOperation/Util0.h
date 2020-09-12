#pragma once
#include <QtGui/QImage>
#include <opencv2/opencv.hpp>

using namespace cv;

class Util
{
public:
	static QImage matToQImage(cv::Mat const& src);
	static QImage matToQImage0(cv::Mat const& src);
	static cv::Mat qimageToMat(QImage const& src);
};

