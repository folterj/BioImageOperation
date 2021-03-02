/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#include <filesystem>
#include <fstream>
#include <regex>
#ifndef _CONSOLE
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrl>
#include <QEventLoop>
#endif
#include "Util.h"


bool Util::contains(string src, string target) {
	return (src.find(target) != string::npos);
}

bool Util::contains(vector<string> source, string target) {
	return (getListIndex(source, target) >= 0);
}

int Util::getListIndex(vector<string> source, string target) {
	int index = -1;
	for (int i = 0; i < source.size(); i++) {
		if (toLower(source[i]) == toLower(target)) {
			index = i;
		}
	}
	return index;
}

string Util::getValueList(vector<string> list) {
	string s;
	for (string item : list) {
		if (s != "") {
			s += ", ";
		}
		s += item;
	}
	return s;
}

bool Util::startsWith(string src, string target) {
	if (src.length() >= target.length()) {
		return (src.substr(0, target.length()) == target);
	}
	return false;
}

bool Util::endsWith(string src, string target) {
	if (src.length() >= target.length()) {
		return (src.substr(src.length() - target.length()) == target);
	}
	return false;
}

vector<string> Util::split(const string s, const string delim, bool removeEmptyEntries) {
	vector<string> parts;
	string part;
	int i = 0;
	int i0 = 0;
	while (i >= 0) {
		i = (int)s.find(delim, i0);
		if (i >= 0) {
			part = s.substr(i0, i - i0);
			if (part != "" || !removeEmptyEntries) {
				parts.push_back(part);
			}
			i0 = i + 1;
		}
	}
	parts.push_back(s.substr(i0, s.length() - i0));
	return parts;
}

vector<string> Util::split(const string s, const vector<string> delims, bool removeEmptyEntries) {
	vector<string> parts;
	string part;
	int i = 0;
	int i0 = 0;
	while (i >= 0) {
		for (string delim : delims) {
			i = (int)s.find(delim, i0);
			if (i >= 0) {
				break;
			}
		}
		if (i >= 0) {
			part = s.substr(i0, i - i0);
			if (part != "" || !removeEmptyEntries) {
				parts.push_back(part);
			}
			i0 = i + 1;
		}
	}
	parts.push_back(s.substr(i0, s.length() - i0));
	return parts;
}

string Util::toLower(string s0) {
	string s;
	for (char c : s0) {
		s += tolower(c);
	}
	return s;
}

string Util::toUpper(string s0) {
	string s;
	for (char c : s0) {
		s += toupper(c);
	}
	return s;
}

string Util::removeQuotes(string s0) {
	string s;
	for (char c : s0) {
		if (c != '\"' && c != '\'') {
			s += c;
		}
	}
	return s;
}

string Util::ltrim(string s0) {
	string s = s0;
	while (s != "" && isspace(s.front())) {
		s = s.substr(1);
	}
	return s;
}

string Util::rtrim(string s0) {
	string s = s0;
	while (s != "" && isspace(s.back())) {
		s = s.substr(0, s.size() - 1);
	}
	return s;
}

string Util::trim(string s0) {
	string s = s0;
	s = ltrim(s);
	s = rtrim(s);
	return s;
}

string Util::replace(string s, string target, string replacement) {
	string output = "";
	int i = 0;
	int i0;

	while (i >= 0) {
		i0 = i;
		i = (int)s.find(target, i);
		if (i >= 0) {
			output += s.substr(i0, i - i0) + replacement;
			i += (int)target.length();
		} else {
			output += s.substr(i0);
		}
	}
	return output;
}

string Util::format(string format, ...) {
	va_list args;
	va_start(args, format);
	const int buflen = vsnprintf(nullptr, 0, format.c_str(), args) + 1;
	va_end(args);
	vector<char> buffer(buflen);
	va_start(args, format);
	vsnprintf(buffer.data(), buflen, format.c_str(), args);
	va_end(args);
	return string(buffer.data());
}

string Util::formatTimespan(int seconds0) {
	string s;
	int hours, minutes;
	int seconds = seconds0;
	hours = seconds / 3600;
	if (hours > 0) {
		s += to_string(hours) + ":";
		seconds %= 3600;
	}
	minutes = seconds / 60;
	if (minutes > 0) {
		s += Util::format("%02d", minutes) + ":";
		seconds %= 60;
	} else {
		s += "0:";
	}
	s += Util::format("%02d", seconds);
	return s;
}

string Util::formatThousands(int x) {
	string s, ns;
	while (x != 0) {
		if (s != "") {
			s = "'" + s;
		}
		ns = to_string(x % 1000);
		s = ns + s;
		x /= 1000;
		if (x != 0) {
			s = string(3 - ns.length(), '0') + s;
		}
	}
	return s;
}

double Util::toDouble(string s) {
	double d = 0;
	size_t stodEnd;
	if (s != "") {
		d = stod(s, &stodEnd);
		if (stodEnd != s.size()) {
			throw invalid_argument("Invalid numeric value: " + s);
		}
	}
	return d;
}

bool Util::isNumeric(string s) {
	double d = 0;
	size_t stodEnd;
	try {
		if (s != "") {
			d = stod(s, &stodEnd);
			return (stodEnd == s.size());
		}
	} catch(...) { }
	return false;
}

bool Util::toBoolean(string s) {
	return (toLower(s) == "true");
}

bool Util::isBoolean(string s) {
	return (toLower(s) == "true" || toLower(s) == "false");
}

int Util::parseFrameTime(string s, double fps) {
	int frames = 0;
	int totalSeconds = 0;

	if (s != "") {
		if (Util::contains(s, ":")) {
			// time format
			for (string part : split(s, ":")) {
				totalSeconds *= 60;
				totalSeconds += stoi(part);
			}
			frames = (int)(totalSeconds * fps);
		} else {
			// frames
			frames = stoi(s);
		}
	}
	return frames;
}

string Util::readText(string filename) {
	ostringstream ss;
	ifstream input;
	input.exceptions(ofstream::failbit | ofstream::badbit);
	try {
		input.open(filename);
		if (input.is_open()) {
			ss << input.rdbuf();
		} else {
			 throw ios_base::failure("Unable to read file " + filename + "\n" + strerror(errno));
		}
	} catch (ios_base::failure e) {
		throw ios_base::failure("Unable to read file " + filename + "\n" + strerror(errno));
	}
	return ss.str();
}

double Util::calcDistance(Point2d point0, Point2d point1) {
	return calcDistance(point0.x, point0.y, point1.x, point1.y);
}

double Util::calcDistance(double x0, double y0, double x1, double y1) {
	return calcDistance(x1 - x0, y1 - y0);
}

double Util::calcDistance(double x, double y) {
	return sqrt(x * x + y * y);
}

double Util::radiansToDegrees(double radAngle) {
	return radAngle / M_PI * 180;
}

double Util::degreesToRadians(double degreeAngle) {
	return degreeAngle / 180 * M_PI;
}

double Util::calcAngle(double dy, double dx) {
	// returns value between -180 and 180
	return radiansToDegrees(atan2(dy, dx));
}

double Util::calcAngleDif(double angle1, double angle2) {
	// for values between -180 and 180
	// returns value between -180 and 180
	// https://stackoverflow.com/questions/1878907/the-smallest-difference-between-2-angles
	return normAngle(angle2 - angle1);
}

double Util::calcShortAngleDif(double angle1, double angle2) {
	// for values between -90 and 90
	// returns value between -90 and 90
	double dangle = angle2 - angle1;
	while (dangle < -90) dangle += 180;
	while (dangle > 90) dangle -= 180;
	return dangle;
}

double Util::normAngle(double angle) {
	// returns value between -180 and 180
	while (angle < -180) angle += 360;
	while (angle > 180) angle -= 360;
	return angle;
}

// https://en.wikipedia.org/wiki/Image_moment
// http://raphael.candelier.fr/?blog=Image%20Moments

double Util::calcMomentsAngle(Moments* moments) {
	// returns value between -90 and 90
	double mu11 = moments->mu11;
	double mu20 = moments->mu20;
	double mu02 = moments->mu02;
	return radiansToDegrees(0.5 * atan2(2 * mu11, mu20 - mu02));
}

double Util::calcMomentsMajorRadius(Moments* moments) {
	double mu11 = moments->mu11 / moments->m00;
	double mu20 = moments->mu20 / moments->m00;
	double mu02 = moments->mu02 / moments->m00;
	double mu2dif = mu20 - mu02;
	double mu2sum = mu20 + mu02;
	return sqrt(2 * (mu2sum + sqrt(mu2dif * mu2dif + 4 * mu11 * mu11)));
}

double Util::calcMomentsMinorRadius(Moments* moments) {
	double mu11 = moments->mu11 / moments->m00;
	double mu20 = moments->mu20 / moments->m00;
	double mu02 = moments->mu02 / moments->m00;
	double mu2dif = mu20 - mu02;
	double mu2sum = mu20 + mu02;
	return sqrt(2 * (mu2sum - sqrt(mu2dif * mu2dif + 4 * mu11 * mu11)));
}

// https://docs.opencv.org/master/dd/d49/tutorial_py_contour_features.html

string Util::getShapeFeatures(vector<Point>* contour, double area, double lengthMajor, double lengthMinor) {
	string csvExtra;
	vector<Point> hull;
	//vector<Point2f> contour2;
	Rect rect;
	Mat areaImage;
	double sizeRatio, ellipsity, circularity, convexity;
	double hullArea, perimeter;

	sizeRatio = lengthMinor / lengthMajor;
	ellipsity = area / (M_PI * (lengthMajor / 2) * (lengthMinor / 2));
	csvExtra += Util::format(",%f,%f", sizeRatio, ellipsity);

	if (!contour->empty()) {
		perimeter = arcLength(*contour, true);
		convexHull(*contour, hull);
		//double epsilon = 0.01 * perimeter;
		//approxPolyDP(hull, contour2, epsilon, true);		// approximation not needed
		//hullArea = contourArea(hull);		// * don't use: returns inaccurate area
		rect = boundingRect(hull);
		// remove offset from hull:
		for (int i = 0; i < hull.size(); i++) {
			hull[i] -= rect.tl();
		}
		areaImage = Mat::zeros(rect.size(), CV_8U);
		fillPoly(areaImage, hull, 1);
		hullArea = countNonZero(areaImage);
		convexity = area / hullArea;
		circularity = area * 4 * M_PI / (perimeter * perimeter);
		csvExtra += Util::format(",%f,%f", circularity, convexity);
	} else {
		csvExtra += ",,";
	}
	return csvExtra;
}

Scalar Util::getHeatScale(double scale) {
	double r = 0;
	double g = 0;
	double b = 0;
	double f = 1;
	double colScale;
	int intScale;
	double floatScale;

	colScale = scale * 4;
	intScale = (int)colScale;
	floatScale = colScale - intScale;
	switch (intScale) {
	case 0:
		// white - yellow
		r = 1;
		g = 1;
		b = 1 - floatScale;
		break;
	case 1:
		// yellow - red
		r = 1;
		g = 1 - floatScale;
		b = 0;
		break;
	case 2:
		// red - blue
		r = 1 - floatScale;
		g = 0;
		b = floatScale;
		break;
	case 3:
		// blue - black
		r = 0;
		g = 0;
		b = 1;
		f = 1 - floatScale;
		break;
	}
	return Scalar(f * b, f * g, f * r);
}

Scalar Util::getRainbowScale(double scale) {
	return hsvToColor(scale, 1, 1);
}

Scalar Util::hsvToColor(double hue1, double saturation, double value) {
	// https://www.rapidtables.com/convert/color/hsv-to-rgb.html
	Scalar rgb1;
	double r, g, b;
	double hue = hue1 * 360;
	double heuf = hue / 60;
	double c = value * saturation;
	double x = c * (1 - abs(fmod(heuf, 2) - 1));
	double m = value - c;
	int huei = (int)heuf;

	switch (huei) {
	case 0: rgb1 = Scalar(c, x, 0); break;
	case 1: rgb1 = Scalar(x, c, 0); break;
	case 2: rgb1 = Scalar(0, c, x); break;
	case 3: rgb1 = Scalar(0, x, c); break;
	case 4: rgb1 = Scalar(x, 0, c); break;
	case 5: rgb1 = Scalar(c, 0, x); break;
	}
	r = rgb1[0] + m;
	g = rgb1[1] + m;
	b = rgb1[2] + m;

	return Scalar(b, g, r);
}

Scalar Util::normColorLightness(Scalar color, double level) {
	double r = color[2];
	double g = color[1];
	double b = color[0];
	double f;
	double level0 = 0.299 * r + 0.587 * g + 0.114 * b;
	if (level0 != 0) {
		f = level / level0;
		if (f > 1) {
			r = 1 - (1 - r) / f;
			g = 1 - (1 - g) / f;
			b = 1 - (1 - b) / f;
		} else {
			r *= f;
			g *= f;
			b *= f;
		}
	}
	return Scalar(b, g, r);
}

Vec<unsigned char, 3> Util::floatToByteColor(Scalar color) {
	return Vec<unsigned char, 3>((unsigned char)(color[0] * 0xFF), (unsigned char)(color[1] * 0xFF), (unsigned char)(color[2] * 0xFF));
}

string Util::getExceptionDetail(exception& e, int level) {
	string s = e.what();
	string inner;
	try {
		std::rethrow_if_nested(e);
	} catch (exception& e) {
		inner = getExceptionDetail(e, level + 1);		// recursive
		if (inner != "") {
			s += " - " + inner;
		}
	} catch (...) { }
	return s;
}

bool Util::isValidImage(Mat* image) {
	if (image) {
		return (!image->empty() && image->dims == 2);
	}
	return false;
}

string Util::getCodecString(int codec) {
	string codecs = "";

	if (codec > 0) {
		for (int i = 0; i < 4; i++) {
			codecs += (char)(codec & 0xFF);
			codec /= 0x100;
		}
	} else {
		codecs = "";
	}
	return codecs;
}

Mat Util::loadImage(string filename) {
	return imread(filename, ImreadModes::IMREAD_UNCHANGED);
}

void Util::saveImage(string filename, Mat* image) {
	imwrite(filename, *image);
}

vector<string> Util::getImageFilenames(string searchpath) {
	vector<string> filenames;
	string filename;
    string path = extractFilePath(searchpath);
    string pattern = extractFileName(searchpath);
	regex rx;

	// very basic * ? pattern matching using regex
	pattern = replace(pattern, ".", "\\.");
	pattern = replace(pattern, "?", ".");
	pattern = replace(pattern, "*", ".*");
    rx = regex(pattern);

    for (const auto& entry : filesystem::directory_iterator(path)) {
        filename = entry.path().string();
        if (regex_match(extractFileName(filename), rx)) {
			filenames.push_back(filename);
        }
    }
	sort(filenames.begin(), filenames.end());
	return filenames;
}

string Util::extractFilePath(string filepath) {
    return filesystem::path(filepath).parent_path().string();
}

string Util::extractFileTitle(string filepath) {
    return filesystem::path(filepath).stem().string();
}

string Util::extractFileName(string filepath) {
    return filesystem::path(filepath).filename().string();
}

string Util::extractFileExtension(string filepath) {
    return filesystem::path(filepath).extension().string();
}

string Util::combinePath(string basepath, string templatepath) {
    return (filesystem::path(basepath) / filesystem::path(templatepath)).string();
}

Size Util::drawText(Mat* image, string text, Point point, HersheyFonts fontFace, double fontScale, Scalar color) {
	Size size = getTextSize(text, fontFace, fontScale, 1, nullptr);
	putText(*image, text, point, fontFace, fontScale, color, 1, LineTypes::LINE_AA);
	return size;
}

void Util::drawAngle(Mat* image, double x, double y, double rad, double angle, Scalar color, bool isArrow) {
	double radAngle = Util::degreesToRadians(angle);
	double frontLength = 1;
	if (isArrow) {
		frontLength = 2;
	}
	int x0 = (int)(x - rad * cos(radAngle));
	int y0 = (int)(y - rad * sin(radAngle));
	int x1 = (int)(x + frontLength * rad * cos(radAngle));
	int y1 = (int)(y + frontLength * rad * sin(radAngle));

	if (isArrow) {
		arrowedLine(*image, Point(x0, y0), Point(x1, y1), color, 1, LineTypes::LINE_AA);
	} else {
		line(*image, Point(x0, y0), Point(x1, y1), color, 1, LineTypes::LINE_AA);
	}
}

#ifndef _CONSOLE
QImage Util::matToQImage(Mat const& source0) {
	Mat source = source0;

	if (source.depth() != CV_8U) {
		source.convertTo(source, CV_8U);
	}
	if (source.channels() == 1) {
		QImage qimage(source.data, source.cols, source.rows, source.step, QImage::Format_Grayscale8);
		return qimage;
	} else {
		QImage qimage(source.data, source.cols, source.rows, source.step, QImage::Format_RGB888);
		return qimage.rgbSwapped();
	}
}

string Util::getUrl(string url) {
	string result;
	try {
		QNetworkAccessManager manager;
		QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(convertToQString(url))));
		QEventLoop loop;
		connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
		if (!reply->error()) {
			QByteArray bytes = reply->readAll();
			QString content = QString::fromUtf8(bytes.data(), bytes.size());
			QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //if (statusCode.isValid()) {
            //	int status = statusCode.toInt();
            //}
			result = content.toStdString();
		}
	} catch (...) { }
	return result;
}

bool Util::openWebLink(string url) {
	try {
		QDesktopServices::openUrl(QUrl(convertToQString(url)));
		return true;
	} catch (...) { }
	return false;
}

int Util::compareVersions(string version1, string version2) {
	int comp = 0;
	vector<string> versions1 = split(version1, ".");
	vector<string> versions2 = split(version2, ".");
	int v1, v2;

	for (int i = 0; i < version1.size() && i < versions2.size(); i++) {
		v1 = stoi(versions1[i]);
		v2 = stoi(versions2[i]);
		if (v2 > v1) {
			comp = 1;
			break;
		} else if (v2 < v1) {
			comp = -1;
			break;
		}
	}
	return comp;
}

QString Util::convertToQString(string s) {
	return QString::fromUtf8(s.c_str());
}
#endif
