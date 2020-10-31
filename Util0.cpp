#include "Util.h"

QImage Util::matToQImage(cv::Mat const& src)
{
    QImage qimage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
    return qimage.rgbSwapped();
}

QImage Util::matToQImage0(cv::Mat const& src)
{
    Mat temp; // make the same cv::Mat
    cvtColor(src, temp, COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage dest((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits(); // enforce deep copy, see documentation of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return dest;
}

cv::Mat Util::qimageToMat(QImage const& src)
{
    Mat tmp(src.height(), src.width(), CV_8UC3, (uchar*)src.bits(), src.bytesPerLine());
    Mat result; // deep copy just in case (my lack of knowledge with open cv)
    cvtColor(tmp, result, COLOR_BGR2RGB);
    return result;
}
