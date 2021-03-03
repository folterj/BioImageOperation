/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <string>
#include <vector>
#ifndef _CONSOLE
#include <QObject>
#include <QImage>
#endif
#include <opencv2/opencv.hpp>
#include "ColorScale.h"

using namespace std;
using namespace cv;


/*
 * General utilities
 */

class Util
#ifndef _CONSOLE
	: public QObject
#endif
{
public:
	static bool contains(string src, string target);
	static bool contains(vector<string> source, string target);
	static int getListIndex(vector<string> source, string target);
	static string getValueList(vector<string> list);
	static bool startsWith(string src, string target);
	static bool endsWith(string src, string target);
	static vector<string> split(const string s, const string delim, bool removeEmptyEntries=false);
	static vector<string> split(const string s, const vector<string> delims, bool removeEmptyEntries=false);
	static string toLower(string s0);
	static string toUpper(string s0);
	static string removeQuotes(string s0);
	static string ltrim(string s0);
	static string rtrim(string s0);
	static string trim(string s0);
	static string replace(string s, string target, string replacement);
	static string format(string format, ...);
	static string formatTimespan(int seconds);
	static string formatThousands(int x);

	static double toDouble(string s);
	static bool isNumeric(string s);
	static bool toBoolean(string s);
	static bool isBoolean(string s);
	static int parseFrameTime(string s, double fps);
	static string readText(string filename);
	static double calcDistance(Point2d point0, Point2d point1);
	static double calcDistance(double x0, double y0, double x1, double y1);
	static double calcDistance(double x, double y);

	static double radiansToDegrees(double radAngle);
	static double degreesToRadians(double degreeAngle);
	static double calcAngle(double dy, double dx);
	static double calcAngleDif(double angle1, double angle2);
	static double calcShortAngleDif(double angle1, double angle2);
	static double normAngle(double angle);
	static double calcMomentsAngle(Moments* moments);
	static double calcMomentsMajorRadius(Moments* moments);
	static double calcMomentsMinorRadius(Moments* moments);
	static string getShapeFeatures(vector<Point>* contour, double area, double lengthMajor, double lengthMinor);

	static Scalar getHeatScale(double scale);
	static Scalar getRainbowScale(double scale);
	static Scalar hsvToColor(double hue1, double saturation, double value);
	static Scalar normColorLightness(Scalar color0, double level = 0.5);
	static Vec<unsigned char, 3> floatToByteColor(Scalar color);

    static string getExceptionDetail(exception& e, int level = 0);
	static Mat loadImage(string filename);
	static void saveImage(string filename, Mat* image);
	static bool isValidImage(Mat* image);
	static string getCodecString(int codec);

    static vector<string> getImageFilenames(string searchpath);
    static string extractFilePath(string filepath);
    static string extractFileTitle(string filepath);
    static string extractFileName(string filepath);
    static string extractFileExtension(string filepath);
    static string combinePath(string basepath, string templatepath);
	static string combineFilename(string basename, string name);

	static Size drawText(Mat* image, string text, Point point, HersheyFonts fontFace, double fontScale, Scalar color);
	static void drawAngle(Mat* image, double x, double y, double rad, double angle, Scalar color, bool isArrow);

#ifndef _CONSOLE
	static QImage matToQImage(Mat const& src);
	static string getUrl(string url);
	static bool openWebLink(string url);
	static int compareVersions(string version1, string version2);
	static QString convertToQString(string s);
#endif
};
