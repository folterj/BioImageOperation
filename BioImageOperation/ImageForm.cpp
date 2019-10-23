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

using namespace System::Drawing::Imaging;
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
	if (bitmapData)
	{
		delete bitmapData;
	}

	if (components)
	{
		delete components;
	}
}

void ImageForm::reset()
{
	displayReady = true;
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
		if (bitmapData)
		{
			delete bitmapData;
			bitmapData = NULL;
		}
		bitmap = matToBitmap(imageBuffer);
		return true;
	}
	else
	{
		return false;
	}
}

Bitmap^ ImageForm::matToBitmap(Mat* image)
{
	Bitmap^ bitmap;
	PixelFormat pixelFormat;
	int width = image->cols;
	int height = image->rows;
	int stride1 = (int)image->step;
	int stride2 = (int)Math::Ceiling(stride1 / 4.0) * 4;
	bool isGrayscale = (image->channels() == 1);
	bool hasAlpha = (image->channels() == 4);
	int bpp = (int)image->elemSize() * 8;
	int offset = 0;

	if (isGrayscale)
	{
		switch (bpp)
		{
		case 1: pixelFormat = PixelFormat::Format1bppIndexed; break;		// bpp < 8 not possible with current implementation of Mat
		case 4: pixelFormat = PixelFormat::Format4bppIndexed; break;		// bpp < 8 not possible with current implementation of Mat
		case 8: pixelFormat = PixelFormat::Format8bppIndexed; break;
		case 16: pixelFormat = PixelFormat::Format16bppGrayScale; break;	// not supported by draw graphics (https://msdn.microsoft.com/en-us/library/system.drawing.graphics.fromimage(v=vs.110).aspx)
		}
	}
	else
	{
		if (hasAlpha)
		{
			switch (bpp)
			{
			case 16: pixelFormat = PixelFormat::Format16bppArgb1555; break;
			case 32: pixelFormat = PixelFormat::Format32bppArgb; break;
			case 64: pixelFormat = PixelFormat::Format64bppArgb; break;
			}
		}
		else
		{
			switch (bpp)
			{
			case 16: pixelFormat = PixelFormat::Format16bppRgb565; break;
			case 24: pixelFormat = PixelFormat::Format24bppRgb; break;
			case 32: pixelFormat = PixelFormat::Format32bppRgb; break;
			case 48: pixelFormat = PixelFormat::Format48bppRgb; break;
			}
		}
	}

	if (stride2 == stride1)
	{
		bitmap = gcnew Bitmap(width, height, stride1, pixelFormat, (IntPtr)image->data);
	}
	else
	{
		// stride is not multiple of 4; convert to image with padding
		bitmapData = new uchar[height * stride2]();		// declare and zet to zeros
		for (int y = 0; y < image->rows; y++)
		{
			memcpy(&bitmapData[offset], image->row(y).data, stride1);
			offset += stride2;
		}
		bitmap = gcnew Bitmap(width, height, stride2, pixelFormat, (IntPtr)bitmapData);
	}

	if (isGrayscale && bpp <= 8)
	{
		// indexed needs (grayscale) palette
		ColorPalette^ palette = bitmap->Palette;
		array<Color>^ entries = palette->Entries;
		int n = entries->Length;
		for (int i = 0; i < n; i++)
		{
			entries[i] = Drawing::Color::FromArgb(i * 0xFF / (n - 1), i * 0xFF / (n - 1), i * 0xFF / (n - 1));
		}
		bitmap->Palette = palette;
	}
	return bitmap;
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
