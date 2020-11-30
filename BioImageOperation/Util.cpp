#define _USE_MATH_DEFINES
#include <math.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrl>
#include <QEventLoop>
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

bool Util::startsWith(string src, string start) {
	if (src.length() >= start.length()) {
		return (src.substr(0, start.length()) == start);
	}
	return false;
}

vector<string> Util::split(const string s, const string delim, bool removeEmptyEntries) {
	vector<string> parts;
	int i = 0;
	int i0 = 0;
	while (i >= 0) {
		i = s.find(delim, i0);
		if (i >= 0) {
			parts.push_back(s.substr(i0, i - i0));
			i0 = i + 1;
		}
	}
	parts.push_back(s.substr(i0, s.length() - i0));
	return parts;
}

vector<string> Util::split(const string s, const vector<string> delims, bool removeEmptyEntries) {
	vector<string> parts;
	int i = 0;
	int i0 = 0;
	while (i >= 0) {
		for (string delim : delims) {
			i = s.find(delim, i0);
			if (i >= 0) {
				break;
			}
		}
		if (i >= 0) {
			parts.push_back(s.substr(i0, i - i0));
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
		i = s.find(target, i);
		if (i >= 0) {
			output += s.substr(i0, i - i0) + replacement;
			i += target.length();
		} else {
			output += s.substr(i0);
		}
	}
	return output;
}

string Util::format(string format, ...) {
	const int buflen = 1000;
	char buffer[buflen];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buflen, format.c_str(), args);
	va_end(args);
	return string(buffer);
}

string Util::formatTimespan(int seconds0) {
	string s;
	int hours, minutes;
	int seconds = seconds0;
	const int buflen = 1000;
	char buffer[buflen];
	hours = seconds / 3600;
	if (hours > 0) {
		s += to_string(hours) + ":";
		seconds %= 3600;
	}
	minutes = seconds / 60;
	if (minutes > 0) {
		s += to_string(minutes) + ":";
		seconds %= 60;
	}
	s += to_string(seconds);
	return s;
}

QString Util::convertToQString(string s) {
	return QString::fromUtf8(s.c_str());
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
	bool ok = (s != "");
	int digits = 0;
	int dots = 0;

	for (char c : s) {
		if (c == '.') {
			dots++;
			if (dots > 1) {
				ok = false;
			}
		} else {
			if (isdigit(c)) {
				digits++;
			} else {
				ok = false;
			}
		}
	}
	return ok;
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
				totalSeconds += toDouble(part);
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
	ifstream input(filename);
	input.exceptions(ifstream::failbit);
	ss << input.rdbuf();
	return ss.str();
}

double Util::calcDistance(double x0, double y0, double x1, double y1) {
	return calcDistance(x1 - x0, y1 - y0);
}

double Util::calcDistance(double x, double y) {
	return sqrt(x * x + y * y);
}

double Util::getMomentsAngle(Moments* moments) {
	return 0.5 * atan2(2 * moments->mu11, moments->mu20 - moments->mu02);
	//return 0.5 * cv::fastAtan2(2 * moments->mu11, moments->mu20 - moments->mu02);
}

double Util::calcAngleDif(double angle1, double angle2) {
	double dangle = angle2 - angle1;
	while (dangle < -M_PI) dangle += 2 * M_PI;
	while (dangle > M_PI) dangle -= 2 * M_PI;
	return dangle;
}

Scalar Util::getLabelColor(int label0) {
	int label;
	int ir, ig, ib;
	double r, g, b;

	if (label0 != 0x10000) {
		label = (label0 % 27);  // 0 ... 26 (27 unique colors)
		ir = label % 3;
		ig = (label / 3) % 3;
		ib = (label / 9) % 3;

		r = 0.25 * (ir + 1);
		g = 0.25 * (ig + 1);
		b = 0.25 * (ib + 1);
	} else {
		r = 0.5;
		g = 0.5;
		b = 0.5;
	}
	return Scalar(b * 0xFF, g * 0xFF, r * 0xFF);
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
	return Scalar((unsigned char)(f * r * 0xFF), (unsigned char)(f * g * 0xFF), (unsigned char)(f * b * 0xFF));
}

Scalar Util::getRainbowScale(double scale) {
	double r = 0;
	double g = 0;
	double b = 0;
	double f = 1;
	double colScale;
	int intScale;
	double floatScale;

	colScale = scale * 6;
	intScale = (int)colScale;
	floatScale = colScale - intScale;
	switch (intScale) {
	case 0:
		// red - yellow
		r = 1 - floatScale;
		g = 1;
		b = 0;
		break;
	case 1:
		// yellow - green
		r = 0;
		g = 1;
		b = floatScale;
		break;
	case 2:
		// green - cyan
		r = 0;
		g = 1 - floatScale;
		b = 1;
		break;
	case 3:
		// cyan - blue
		r = floatScale;
		g = 0;
		b = 1;
		break;
	case 4:
		// blue - purple
		r = 1;
		g = 0;
		b = 1 - floatScale;
		break;
	case 5:
		// purple - red/black
		r = 1;
		g = floatScale;
		b = 0;
		f = 1 - floatScale;
		break;
	}
	return Scalar((unsigned char)(f * r * 0xFF), (unsigned char)(f * g * 0xFF), (unsigned char)(f * b * 0xFF));
}

Scalar Util::bgrtoScalar(BGR bgr) {
	return Scalar(bgr.b, bgr.g, bgr.r);
}

string Util::getExceptionDetail(exception e, int level) {
	string s = e.what();
	string inner;
	try {
		std::rethrow_if_nested(e);
	} catch (exception e) {
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

vector<string> Util::getImageFilenames(string searchPath) {
	vector<string> filenames;
	string filename;
	string path = extractFilePath(searchPath);
	string pattern = extractFileName(searchPath);
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

string Util::extractFilePath(string path) {
	string filepath = "";
	vector<string> parts = split(path, vector<string>{"\\", "/"});
	for (int i = 0; i < (int)parts.size() - 1; i++) {
		filepath += parts[i];
		filepath += "\\";
	}
	return filepath;
}

string Util::extractTitle(string path) {
	return extractFileTitle(extractFileName(path));
}

string Util::extractFileName(string path) {
	string filename = path;
	vector<string> parts = split(path, vector<string>{"\\", "/"});
	if (parts.size() > 1) {
		filename = parts[parts.size() - 1];
	}
	return filename;
}

string Util::extractFileTitle(string filename) {
	string fileTitle = "";
	vector<string> parts = split(filename, ".");
	for (int i = 0; i < parts.size() - 1; i++) {
		if (i > 0) {
			fileTitle += ".";
		}
		fileTitle += parts[i];
	}
	return fileTitle;
}

string Util::extractFileExtension(string filename) {
	string fileExtension = filename;
	vector<string> parts = split(fileExtension, ".");
	if (parts.size() > 1) {
		fileExtension = parts[parts.size() - 1];
	}
	return fileExtension;
}

string Util::combinePath(string basepath, string templatePath) {
	if (basepath == "" || Util::contains(templatePath, ":")) {
		return templatePath;
	} else {
		return (filesystem::path(basepath) / filesystem::path(templatePath)).string();
	}
}

QImage Util::matToQImage(cv::Mat const& source) {
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
			if (statusCode.isValid()) {
				int status = statusCode.toInt();
			}
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
