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
#include "ColorScale.h"

using namespace std;
using namespace cv;


/*
 * General utilities
 */
 
class Util
{
public:
	static string numPadZeros(int number, string format);
	static bool contains(string src, string target);
	template <typename Out>
	static void split(const string& s, const string& delim, Out result);
	static vector<string> split(const string& s, const string& delim);
	static string toLower(string s);
	static string toUpper(string s);
	static string removeQuotes(string s);
	// trim from start (in place)
	static void ltrim(string& s);
	// trim from end (in place)
	static void rtrim(string& s);
	// trim from both ends (in place)
	static void trim(string& s);
	// trim from start (copying)
	static string ltrim_copy(string s);
	// trim from end (copying)
	static string rtrim_copy(string s);
	// trim from both ends (copying)
	static string trim_copy(string s);

	static double toDouble(string s);
	static bool isNumeric(string s);
	static bool toBoolean(string s);
	static bool isBoolean(string s);
	static int getListIndex(vector<string> source, string target);
	static bool listContains(vector<string> source, string target);
	static int parseFrameTime(string s, double fps);
	static double calcDistance(double x0, double y0, double x1, double y1);
	static double calcDistance(double x, double y);

	static double getMomentsAngle(Moments* moments);
	static double calcAngleDif(double angle1, double angle2);

	static Scalar getLabelColor(int label0);
	static Scalar getHeatScale(double scale);
	static Scalar getRainbowScale(double scale);
	static Scalar bgrtoScalar(BGR bgr);

	static Mat loadImage(string fileName);
	static void saveImage(string fileName, Mat* image);
	static bool isValidImage(Mat* image);
	static string getCodecString(int codec);

	static vector<string> getImageFileNames(string searchPath);
	static string extractFilePath(string path);
	static string extractTitle(string path);
	static string extractFileName(string path);
	static string extractFileTitle(string fileName);
	static string extractFileExtension(string fileName);
	static string combinePath(string basePath, string templatePath);

	static QImage matToQImage(cv::Mat const& src);
};
