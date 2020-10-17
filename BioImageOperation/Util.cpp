#include "Util.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <filesystem>


string Util::numPadZeros(int number, string format) {
	const int buflen = 1000;
	char buffer[buflen];
	snprintf(buffer, buflen, format.c_str(), number);
	return string(buffer);
}

bool Util::contains(string src, string target) {
	return (src.find(target) != string::npos);
}

bool Util::contains(string src, string target) {
	return (src.find(target) != string::npos);
}

template <typename Out>
void Util::split(const string& s, const string& delim, Out result) {
	istringstream iss(s);
	string item;
	while (getline(iss, item, delim)) {
		*result++ = item;
	}
}

vector<string> Util::split(const string& s, const string& delim) {
	vector<string> elems;
	split(s, delim, back_inserter(elems));
	return elems;
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

int Util::getListIndex(vector<string> source, string target)
{
	int index = -1;
	auto item = find(begin(source), end(source), target);
	if (item != end(source)) {
		index = distance(begin(source), item);
	}
	return index;
}

bool Util::listContains(vector<string> source, string target)
{
	return (getListIndex(source, target) != end(source));
}

int Util::parseFrameTime(string s, double fps)
{
	int frames = 0;
	int totalSeconds = 0;

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

		r = 0.25 * (1 + ir);
		g = 0.25 * (1 + ig);
		b = 0.25 * (1 + ib);
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

Mat Util::loadImage(string fileName)
{
	return imread(fileName, ImreadModes::IMREAD_UNCHANGED);
}

void Util::saveImage(string fileName, Mat* image)
{
	imwrite(fileName, *image);
}

vector<string> Util::getImageFileNames(string searchPath)
{
	vector<string> fileNames;
	string fileName;
	string path = extractFilePath(searchPath);
	string pattern = extractFileName(searchPath);

	for (const auto& entry : filesystem::directory_iterator(path)) {
		fileName = entry.path().string();
		if (fileName._Starts_with(pattern)) {
			fileNames.push_back(fileName);
		}
	}
	sort(fileNames.begin(), fileNames.end());
	return fileNames;
}

string Util::extractFilePath(string path)
{
	string filePath = "";
	vector<string> parts = split(path, "\\");
	for (int i = 0; i < (int)parts.size() - 1; i++) {
		filePath += parts[i];
		filePath += "\\";
	}
	return filePath;
}

string Util::extractTitle(string path)
{
	return extractFileTitle(extractFileName(path));
}

string Util::extractFileName(string path)
{
	string fileName = path;
	vector<string> parts = split(path, "\\");
	if (parts.size() > 1) {
		fileName = parts[parts.size() - 1];
	}
	return fileName;
}

string Util::extractFileTitle(string fileName)
{
	string fileTitle = "";
	vector<string> parts = split(fileName, ".");
	for (int i = 0; i < parts.size() - 1; i++) {
		if (i > 0) {
			fileTitle += ".";
		}
		fileTitle += parts[i];
	}
	return fileTitle;
}

string Util::extractFileExtension(string fileName)
{
	string fileExtension = fileName;
	vector<string> parts = split(fileExtension, ".");
	if (parts.size() > 1) {
		fileExtension = parts[parts.size() - 1];
	}
	return fileExtension;
}

string Util::combinePath(string basePath, string templatePath)
{
	if (basePath == "" || Util::contains(templatePath, ":")) {
		return templatePath;
	} else {
		return (filesystem::path(basePath) / filesystem::path(templatePath)).string();
	}
}

QImage Util::matToQImage(cv::Mat const& src)
{
	QImage qimage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
	return qimage.rgbSwapped();
}
