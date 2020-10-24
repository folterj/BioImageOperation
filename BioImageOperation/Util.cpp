#include "Util.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <filesystem>


bool Util::contains(string src, string target) {
	return (src.find(target) != string::npos);
}

bool Util::contains(vector<string> source, string target)
{
	auto item = find(begin(source), end(source), target);
	return (item != end(source));
}

int Util::getListIndex(vector<string> source, string target)
{
	int index = -1;
	auto item = find(begin(source), end(source), target);
	if (item != end(source)) {
		index = distance(begin(source), item);
	}
	return index;
}

vector<string> Util::split(const string& s, const string& delim, bool removeEmptyEntries) {
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

vector<string> Util::split(const string& s, const vector<string>& delims, bool removeEmptyEntries) {
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

string Util::toLower(string s) {
	string s2;
	transform(s.begin(), s.end(), s2.begin(), [](unsigned char c) { return tolower(c); });
	return s2;
}

string Util::toUpper(string s) {
	string s2;
	transform(s.begin(), s.end(), s2.begin(), [](unsigned char c) { return toupper(c); });
	return s2;
}

string Util::removeQuotes(string s) {
	string s2;
	for (char c : s) {
		if (c != '\"' && c != '\'') {
			s2 += c;
		}
	}
	return s2;
}

void Util::ltrim(string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

void Util::rtrim(string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

void Util::trim(string& s) {
	ltrim(s);
	rtrim(s);
}

string Util::ltrim_copy(string s) {
	// *** in-place operation!
	ltrim(s);
	return s;
}

string Util::rtrim_copy(string s) {
	// *** in-place operation!
	rtrim(s);
	return s;
}

string Util::trim_copy(string s) {
	// *** in-place operation!
	trim(s);
	return s;
}

string Util::numPadZeros(int number, string format) {
	const int buflen = 1000;
	char buffer[buflen];
	snprintf(buffer, buflen, format.c_str(), number);
	return string(buffer);
}

double Util::toDouble(string s)
{
	if (isNumeric(s))
	{
		return stod(s);
	}
	return 0;
}

bool Util::isNumeric(string s)
{
	try {
		stod(s);
		return true;
	} catch (...) {
	}
	return false;
}

bool Util::toBoolean(string s)
{
	return (toLower(s) == "true");
}

bool Util::isBoolean(string s)
{
	return (toLower(s) == "true" || toLower(s) == "false");
}

int Util::parseFrameTime(string s, double fps)
{
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

double Util::calcDistance(double x0, double y0, double x1, double y1)
{
	return calcDistance(x1 - x0, y1 - y0);
}

double Util::calcDistance(double x, double y)
{
	return sqrt(x * x + y * y);
}

double Util::getMomentsAngle(Moments* moments)
{
	return 0.5 * atan2(2 * moments->mu11, moments->mu20 - moments->mu02);
	//return 0.5 * cv::fastAtan2(2 * moments->mu11, moments->mu20 - moments->mu02);
}

double Util::calcAngleDif(double angle1, double angle2)
{
	double dangle = angle2 - angle1;
	while (dangle < -M_PI) dangle += 2 * M_PI;
	while (dangle > M_PI) dangle -= 2 * M_PI;
	return dangle;
}

Scalar Util::getLabelColor(int label0)
{
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

Scalar Util::getHeatScale(double scale)
{
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
	switch (intScale)
	{
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

Scalar Util::getRainbowScale(double scale)
{
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
	switch (intScale)
	{
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

Scalar Util::bgrtoScalar(BGR bgr)
{
	return Scalar(bgr.b, bgr.g, bgr.r);
}

bool Util::isValidImage(Mat* image)
{
	if (image)
	{
		return (!image->empty() && image->dims == 2);
	}
	return false;
}

string Util::getCodecString(int codec)
{
	string codecs = "";
	
	if (codec > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			codecs += (char)(codec & 0xFF);
			codec /= 0x100;
		}
	}
	else
	{
		codecs = "";
	}
	return codecs;
}

Mat Util::loadImage(string filename)
{
	return imread(filename, ImreadModes::IMREAD_UNCHANGED);
}

void Util::saveImage(string filename, Mat* image)
{
	imwrite(filename, *image);
}

vector<string> Util::getImageFilenames(string searchPath)
{
	vector<string> filenames;
	string filename;
	string path = extractFilePath(searchPath);
	string pattern = extractFileName(searchPath);

	for (const auto& entry : filesystem::directory_iterator(path)) {
		filename = entry.path().string();
		if (extractFileName(filename)._Starts_with(pattern)) {
			filenames.push_back(filename);
		}
	}
	sort(filenames.begin(), filenames.end());
	return filenames;
}

string Util::extractFilePath(string path)
{
	string filepath = "";
	vector<string> parts = split(path, vector<string>{"\\", "/"});
	for (int i = 0; i < (int)parts.size() - 1; i++) {
		filepath += parts[i];
		filepath += "\\";
	}
	return filepath;
}

string Util::extractTitle(string path)
{
	return extractFileTitle(extractFileName(path));
}

string Util::extractFileName(string path)
{
	string filename = path;
	vector<string> parts = split(path, vector<string>{"\\", "/"});
	if (parts.size() > 1) {
		filename = parts[parts.size() - 1];
	}
	return filename;
}

string Util::extractFileTitle(string filename)
{
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

string Util::extractFileExtension(string filename)
{
	string fileExtension = filename;
	vector<string> parts = split(fileExtension, ".");
	if (parts.size() > 1) {
		fileExtension = parts[parts.size() - 1];
	}
	return fileExtension;
}

string Util::combinePath(string basepath, string templatePath)
{
	if (basepath == "" || Util::contains(templatePath, ":")) {
		return templatePath;
	} else {
		return (filesystem::path(basepath) / filesystem::path(templatePath)).string();
	}
}

QImage Util::matToQImage(cv::Mat const& src)
{
	QImage qimage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
	return qimage.rgbSwapped();
}
