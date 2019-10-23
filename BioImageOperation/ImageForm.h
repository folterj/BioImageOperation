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

#pragma once

#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace cv;


/*
 * Windows Form for displaying images - optimised for performance
 */

namespace BioImageOperation {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::Diagnostics;

	public ref class ImageForm : public System::Windows::Forms::Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
	private: System::ComponentModel::IContainer^  components;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip;
	private: System::Windows::Forms::ToolStripMenuItem^  saveToolStripMenuItem;

		int title;
		Mat* imageBuffer;
		uchar* bitmapData;
		Bitmap^ bitmap;
		int swidth = 0;
		int sheight = 0;
		int displayCount = 0;
		int displayFps = 0;
		bool displayReady = true;

	public:
		ImageForm(void);
		void reset();
		void setTitle(int title);
		void updateTitle();
		bool setImage(Mat* image);
		Bitmap^ matToBitmap(Mat* image);
		void show();
		void updateFps();

	protected:
		~ImageForm();
		virtual void OnPaint(PaintEventArgs^ e) override;

	private:
		System::Void ImageForm_Resize(System::Object^  sender, System::EventArgs^  e);
		System::Void ImageForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
		System::Void saveToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(ImageForm::typeid));
			this->contextMenuStrip = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->saveToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->contextMenuStrip->SuspendLayout();
			this->SuspendLayout();
			// 
			// contextMenuStrip
			// 
			this->contextMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->saveToolStripMenuItem });
			this->contextMenuStrip->Name = L"contextMenuStrip";
			this->contextMenuStrip->Size = System::Drawing::Size(99, 26);
			// 
			// saveToolStripMenuItem
			// 
			this->saveToolStripMenuItem->Name = L"saveToolStripMenuItem";
			this->saveToolStripMenuItem->Size = System::Drawing::Size(98, 22);
			this->saveToolStripMenuItem->Text = L"Save";
			this->saveToolStripMenuItem->Click += gcnew System::EventHandler(this, &ImageForm::saveToolStripMenuItem_Click);
			// 
			// ImageForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(814, 461);
			this->ContextMenuStrip = this->contextMenuStrip;
			this->DoubleBuffered = true;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"ImageForm";
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Show;
			this->Text = L"BIO Image";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &ImageForm::ImageForm_FormClosing);
			this->Resize += gcnew System::EventHandler(this, &ImageForm::ImageForm_Resize);
			this->contextMenuStrip->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion

	};
}
