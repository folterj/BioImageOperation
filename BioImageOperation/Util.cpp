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

#include "Util.h"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net;
using namespace System::IO;
using namespace System::Windows::Forms;
using namespace Runtime::InteropServices;


double Util::toDouble(System::String^ s)
{
	if (isNumeric(s))
	{
		return double::Parse(s);
	}
	return 0;
}

bool Util::isNumeric(System::String^ s)
{
	double x;

	return double::TryParse(s, x);
}

bool Util::isBoolean(System::String^ s)
{
	bool b;

	return bool::TryParse(s, b);
}

int Util::parseFrameTime(System::String^ s, double fps)
{
	int frames = 0;
	int count;

	if (s->Contains(":"))
	{
		// time format
		count = s->Split(':')->Length - 1;
		if (count == 1)
		{
			s = "0:" + s;
		}
		frames = (int)(TimeSpan::Parse(s).TotalSeconds * fps);
	}
	else
	{
		// frames
		int::TryParse(s, frames);
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
	while (dangle < -Math::PI) dangle += 2 * Math::PI;
	while (dangle > Math::PI) dangle -= 2 * Math::PI;
	return dangle;
}

Scalar Util::getLabelColor(int label0)
{
	int label;
	int ir, ig, ib;
	double r, g, b;

	if (label0 != 0x10000)
	{
		label = (label0 % 27);  // 0 ... 26 (27 unique colors)
		ir = label % 3;
		ig = (label / 3) % 3;
		ib = (label / 9) % 3;

		r = 0.25f * (1 + ir);
		g = 0.25f * (1 + ig);
		b = 0.25f * (1 + ib);
	}
	else
	{
		r = 0.5f;
		g = 0.5f;
		b = 0.5f;
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

	colScale = (float)scale * 4;
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

	colScale = (float)scale * 6;
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

std::string Util::stdString(System::String^ s)
{
	const char* cstr = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	std::string s2 = cstr;
	Marshal::FreeHGlobal(IntPtr((void*)cstr));
	return s2;
}

System::String^ Util::netString(std::string s)
{
	return gcnew System::String(s.c_str());
}

std::vector<std::string> Util::stdStringVector(array<System::String^>^ list)
{
	std::vector<std::string> vec;

	for each (System::String^ s in list)
	{
		vec.push_back(stdString(s));
	}
	return vec;
}

System::String^ Util::getExceptionDetail(System::Exception^ e)
{
	System::String^ s = e->Message;
	System::Exception^ innerException = e->InnerException;

	if (innerException)
	{
		s += " - " + getExceptionDetail(innerException);
	}
	return s;
}

bool Util::isValidImage(Mat* image)
{
	if (image)
	{
		return (!image->empty() && image->dims == 2);
	}
	return false;
}

System::String^ Util::getCodecString(int codec)
{
	System::String^ codecs = "";
	
	if (codec > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			codecs += (Char)(codec & 0xFF);
			codec /= 0x100;
		}
	}
	else
	{
		codecs = "";
	}
	return codecs;
}

Mat Util::loadImage(System::String^ fileName)
{
	return imread(stdString(fileName), ImreadModes::IMREAD_UNCHANGED);
}

void Util::saveImage(System::String^ fileName, Mat* image)
{
	imwrite(stdString(fileName), *image);
}



array<System::String^>^ Util::getImageFileNames(System::String^ searchPath)
{
	System::String^ path = extractFilePath(searchPath);
	System::String^ pattern = extractFileName(searchPath);
	array<System::String^>^ fileNames = Directory::GetFiles(path, pattern);
	Array::Sort(fileNames);
	return fileNames;
}

System::String^ Util::extractFilePath(System::String^ path)
{
	System::String^ filePath = "";
	array<System::String^>^ parts = path->Split('\\');
	for (int i = 0; i < parts->Length - 1; i++)
	{
		filePath += parts[i];
		filePath += "\\";
	}
	return filePath;
}

System::String^ Util::extractTitle(System::String^ path)
{
	return extractFileTitle(extractFileName(path));
}

System::String^ Util::extractFileName(System::String^ path)
{
	System::String^ fileName = path;
	array<System::String^>^ parts = path->Split('\\');
	if (parts->Length > 1)
	{
		fileName = parts[parts->Length - 1];
	}
	return fileName;
}

System::String^ Util::extractFileTitle(System::String^ fileName)
{
	System::String^ fileTitle = "";
	array<System::String^>^ parts = fileName->Split('.');
	for (int i = 0; i < parts->Length - 1; i++)
	{
		if (i > 0)
		{
			fileTitle += ".";
		}
		fileTitle += parts[i];
	}
	return fileTitle;
}

System::String^ Util::extractFileExtension(System::String^ fileName)
{
	System::String^ fileExtension = fileName;
	array<System::String^>^ parts = fileExtension->Split('.');
	if (parts->Length > 1)
	{
		fileExtension = parts[parts->Length - 1];
	}
	return fileExtension;
}

System::String^ Util::combinePath(System::String^ basePath, System::String^ templatePath)
{
	if (basePath == "" || templatePath->Contains(":"))
	{
		return templatePath;
	}
	else
	{
		return Path::Combine(basePath, templatePath);
	}
}

System::String^ Util::getUrl(System::String^ url)
{
	System::String^ content = "";

	WebRequest^ request = WebRequest::Create(url);
	request->UseDefaultCredentials = true;
	request->Timeout = 30000;	// takes max approx 2.5 seconds
	((HttpWebRequest^)request)->Accept = "*/*";
	((HttpWebRequest^)request)->UserAgent = "compatible";	// essential!
	WebResponse^ response = request->GetResponse();
	Stream^ responseStream = response->GetResponseStream();
	StreamReader^ reader = gcnew StreamReader(responseStream);
	content = reader->ReadToEnd();
	reader->Close();
	responseStream->Close();
	response->Close();

	return content;
}

bool Util::openWebLink(System::String^ link)
{
	try
	{
		Process::Start(link);
		return true;
	}
	catch (System::Exception^ e)
	{
		Clipboard::SetText(link);
		MessageBox::Show("Error: " + e->Message + "\nPlease use your preferred browser to navigate to this link manually\n(This link has been copied to the clipboard - Select Paste in your browser address bar)", "BIO Update");
	}
	return false;
}

int Util::compareVersions(System::String^ version1, System::String^ version2)
{
	int comp = 0;
	array<System::String^>^ versions1 = version1->Split('.');
	array<System::String^>^ versions2 = version2->Split('.');
	int v1, v2;

	for (int i = 0; i < version1->Length && i < versions2->Length; i++)
	{
		int::TryParse(versions1[i], v1);
		int::TryParse(versions2[i], v2);

		if (v2 > v1)
		{
			comp = 1;
			break;
		}
		else if (v2 < v1)
		{
			comp = -1;
			break;
		}
	}
	return comp;
}
