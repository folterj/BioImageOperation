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

#include "ImageForm.h"
#include "Constants.h"
#include "Util.h"

using namespace BioImageOperation;


ImageForm::ImageForm(void)
{
	InitializeComponent();
}

ImageForm::~ImageForm()
{
	if (imageBuffer)
	{
		delete imageBuffer;
	}

	if (components)
	{
		delete components;
	}
}

void ImageForm::updateTitle()
{
	System::String^ s = System::String::Format("BIO Image {0}", title);

	if (swidth != 0 && sheight != 0)
	{
		s += System::String::Format(" {0}x{1}", swidth, sheight);
	}
	if (displayFps != 0)
	{
		s += System::String::Format(" @{0:F0}fps", displayFps);
	}
	Text = s;
}

void ImageForm::setTitle(int title)
{
	this->title = title;
	updateTitle();
}

bool ImageForm::setImage(Mat* image)
{
	if (displayReady)
	{
		displayReady = false;
		if (!imageBuffer)
		{
			imageBuffer = new Mat();
		}
		image->copyTo(*imageBuffer);
		bitmap = Util::matToBitmap(imageBuffer);

		return true;
	}
	else
	{
		return false;
	}
}

void ImageForm::show()
{
	if (!Visible)
	{
		Show();
	}
	Invalidate();
}

void ImageForm::OnPaint(PaintEventArgs^ e)
{
	Form::OnPaint(e);

	if (bitmap != nullptr)
	{
		int dwidth, dheight;
		int x, y;

		swidth = bitmap->Width;
		sheight = bitmap->Height;

		int dwidth0 = ClientRectangle.Width;
		int dheight0 = ClientRectangle.Height;

		double widthreduct = (double)swidth / dwidth0;
		double heightreduct = (double)sheight / dheight0;

		if (widthreduct > heightreduct)
		{
			dwidth = dwidth0;
			dheight = (int)(sheight / widthreduct);
			x = 0;
			y = (dheight0 - dheight) / 2;
		}
		else
		{
			dwidth = (int)(swidth / heightreduct);
			dheight = dheight0;
			x = (dwidth0 - dwidth) / 2;
			y = 0;
		}

		e->Graphics->DrawImage(bitmap, x, y, dwidth, dheight);
		displayCount++;
		updateTitle();
	}
	displayReady = true;
}

void ImageForm::updateFps()
{
	displayFps = displayCount;
	displayCount = 0;
}

System::Void ImageForm::ImageForm_Resize(System::Object^  sender, System::EventArgs^  e)
{
	// Force redraw of entire view
	Invalidate();
}

System::Void ImageForm::ImageForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	e->Cancel = true;
	Hide();
}

System::Void ImageForm::saveToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	SaveFileDialog fileDialog;
	System::String^ filename;
	System::String^ extension;
	int extPos;

	try
	{
		if (fileDialog.ShowDialog() == ::DialogResult::OK)
		{
			filename = fileDialog.FileName;
			extPos = filename->LastIndexOf(".");
			if (extPos < 0)
			{
				extension = Util::netString(Constants::defaultImageExtension);
				if (!extension->StartsWith("."))
				{
					extension = "." + extension;
				}
				filename += extension;
			}
			Util::saveImage(filename, imageBuffer);
		}
	}
	catch (cv::Exception& e)
	{
		// opencv exception
		MessageBox::Show(Util::netString(e.err), "Image save error");
	}
	catch (System::Exception^ e)
	{
		MessageBox::Show(Util::getExceptionDetail(e), "Image save error");
	}
}
