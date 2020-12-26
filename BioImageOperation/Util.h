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
#include <QImage>
#include <opencv2/opencv.hpp>
#include "ColorScale.h"

using namespace std;
using namespace cv;


/*
 * General utilities
 */

class Util : public QObject
{
public:
	static bool contains(string src, string target);
	static bool contains(vector<string> source, string target);
	static int getListIndex(vector<string> source, string target);
	static string getValueList(vector<string> list);
	static bool startsWith(string src, string start);
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
	static QString convertToQString(string s);

	static double toDouble(string s);
	static bool isNumeric(string s);
	static bool toBoolean(string s);
	static bool isBoolean(string s);
	static int parseFrameTime(string s, double fps);
	static string readText(string filename);
	static double calcDistance(double x0, double y0, double x1, double y1);
	static double calcDistance(double x, double y);

	static double radiansToDegrees(double radAngle);
	static double degreesToRadians(double degreeAngle);
	static double calcAngle(double dy, double dx);
	static double calcMomentsAngle(Moments* moments);
	static double calcMomentsMajorRadius(Moments* moments);
	static double calcMomentsMinorRadius(Moments* moments);
	static double calcAngleDif(double angle1, double angle2);
	static double calcShortAngleDif(double angle1, double angle2);

	static Scalar getLabelColor(int label0);
	static Scalar getHeatScale(double scale);
	static Scalar getRainbowScale(double scale);
	static Scalar bgrtoScalar(BGR bgr);

	static string getExceptionDetail(exception e, int level = 0);
	static Mat loadImage(string filename);
	static void saveImage(string filename, Mat* image);
	static bool isValidImage(Mat* image);
	static string getCodecString(int codec);

	static vector<string> getImageFilenames(string searchPath);
	static string extractFilePath(string path);
	static string extractTitle(string path);
	static string extractFileName(string path);
	static string extractFileTitle(string filename);
	static string extractFileExtension(string filename);
	static string combinePath(string basepath, string templatePath);

	static Size drawText(Mat* image, string text, Point point, HersheyFonts fontFace, double fontScale, Scalar color);
	static QImage matToQImage(Mat const& src);

	static string getUrl(string url);
	static bool openWebLink(string url);
	static int compareVersions(string version1, string version2);
};
