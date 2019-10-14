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

#using <mscorlib.dll>
#include "Observer.h"
#include "ImageForm.h"
#include "InfoForm.h"
#include "ScriptProcessing.h"
#include "BenchMark.h"
#include "Constants.h"

#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace cv;


/*
 * Windows Form for main UI including script view
 */

namespace BioImageOperation {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form, Observer
	{
	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
	private: System::Windows::Forms::StatusStrip^  statusStrip;
	private: System::Windows::Forms::ToolStripProgressBar^  progressBar;
	private: System::Windows::Forms::ToolStripStatusLabel^  statusLabel;
	private: System::Windows::Forms::RichTextBox^  scriptText;
	private: System::Windows::Forms::Button^  processButton;
	private: System::Windows::Forms::Button^  abortButton;
	private: System::Windows::Forms::Button^  testButton;

	private: System::Windows::Forms::MenuStrip^  mainMenuStrip;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  clearToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  openToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator;
	private: System::Windows::Forms::ToolStripMenuItem^  saveToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  saveAsToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  viewToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  scripthelpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator5;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  checkUpdatesToolStripMenuItem;

	public:
		ScriptProcessing^ scriptProcessing = gcnew ScriptProcessing(this);
		array<ImageForm^>^ imageForms = gcnew array<ImageForm^>(Constants::nDisplays);
		array<InfoForm^>^ infoForms = gcnew array<InfoForm^>(Constants::nInfoWindows);
		InfoForm operationInfoForm;
		BenchMark* benchMark = new BenchMark(this);
		System::String^ filePath = "";
		Stopwatch stopwatch;
		Timers::Timer secondsTimer;
		int processCount = 0;
		int processFps = 0;

		MainForm(void);
		void setFilePath(System::String^ filePath);
		void clearInput();
		void open();
		void save();
		void updateUI(bool start);
		void checkUpdates();

		virtual void resetProgressTimer();

		virtual void resetUI();
		virtual void resetImages();
		delegate void ResetUIDelegate();
		ResetUIDelegate^ resetUIDelegate = gcnew ResetUIDelegate(this, &MainForm::resetUIDelegateMethod);
		void resetUIDelegateMethod();

		virtual void clearStatus();
		virtual void showStatus(int i);
		virtual void showStatus(int i, int tot);
		virtual void showStatus(System::String^ label, int i, int tot, bool showFrameProgress);
		virtual void showStatus(System::String^ status, double progress);
		delegate void ShowStatusDelegate(System::String^ status, double progress);
		ShowStatusDelegate^ showStatusDelegate = gcnew ShowStatusDelegate(this, &MainForm::showStatusDelegateMethod);
		void showStatusDelegateMethod(System::String^ status, double progress);

		virtual void displayImage(Mat* image, int displayi);
		delegate void DisplayImageDelegate(int displayi);
		DisplayImageDelegate^ displayImageDelegate = gcnew DisplayImageDelegate(this, &MainForm::displayImageDelegateMethod);
		void displayImageDelegateMethod(int displayi);

		virtual void showInfo(System::String^ info, int displayi);
		delegate void ShowInfoDelegate(System::String^ info, int displayi);
		ShowInfoDelegate^ showInfoDelegate = gcnew ShowInfoDelegate(this, &MainForm::showInfoDelegateMethod);
		void showInfoDelegateMethod(System::String^ info, int displayi);

		virtual void showErrorMessage(System::String^ message);
		delegate void ShowErrorMessageDelegate(System::String^ message);
		ShowErrorMessageDelegate^ showErrorMessageDelegate = gcnew ShowErrorMessageDelegate(this, &MainForm::showErrorMessageDelegateMethod);
		void showErrorMessageDelegateMethod(System::String^ message);

	protected:
		~MainForm();

	private:
		void OnElapsed(System::Object ^sender, System::Timers::ElapsedEventArgs ^e);
		System::Void MainForm_Shown(System::Object^  sender, System::EventArgs^  e);
		System::Void clearToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void saveToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void saveAsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void scripthelpToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void checkUpdatesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void testButton_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void processButton_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void abortButton_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void MainForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
			this->statusStrip = (gcnew System::Windows::Forms::StatusStrip());
			this->progressBar = (gcnew System::Windows::Forms::ToolStripProgressBar());
			this->statusLabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->scriptText = (gcnew System::Windows::Forms::RichTextBox());
			this->processButton = (gcnew System::Windows::Forms::Button());
			this->abortButton = (gcnew System::Windows::Forms::Button());
			this->testButton = (gcnew System::Windows::Forms::Button());
			this->mainMenuStrip = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->clearToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripSeparator = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->saveToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->saveAsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->viewToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->scripthelpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->checkUpdatesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripSeparator5 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip->SuspendLayout();
			this->mainMenuStrip->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip
			// 
			this->statusStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) { this->progressBar, this->statusLabel });
			this->statusStrip->Location = System::Drawing::Point(0, 539);
			this->statusStrip->Name = L"statusStrip";
			this->statusStrip->Size = System::Drawing::Size(584, 22);
			this->statusStrip->TabIndex = 0;
			// 
			// progressBar
			// 
			this->progressBar->Name = L"progressBar";
			this->progressBar->Size = System::Drawing::Size(100, 16);
			// 
			// statusLabel
			// 
			this->statusLabel->Name = L"statusLabel";
			this->statusLabel->Size = System::Drawing::Size(467, 17);
			this->statusLabel->Spring = true;
			// 
			// scriptText
			// 
			this->scriptText->AcceptsTab = true;
			this->scriptText->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->scriptText->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->scriptText->Location = System::Drawing::Point(0, 27);
			this->scriptText->Name = L"scriptText";
			this->scriptText->Size = System::Drawing::Size(584, 480);
			this->scriptText->TabIndex = 1;
			this->scriptText->Text = L"";
			// 
			// processButton
			// 
			this->processButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->processButton->Location = System::Drawing::Point(428, 513);
			this->processButton->Name = L"processButton";
			this->processButton->Size = System::Drawing::Size(75, 23);
			this->processButton->TabIndex = 2;
			this->processButton->Text = L"Process";
			this->processButton->UseVisualStyleBackColor = true;
			this->processButton->Click += gcnew System::EventHandler(this, &MainForm::processButton_Click);
			// 
			// abortButton
			// 
			this->abortButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->abortButton->Enabled = false;
			this->abortButton->Location = System::Drawing::Point(509, 513);
			this->abortButton->Name = L"abortButton";
			this->abortButton->Size = System::Drawing::Size(75, 23);
			this->abortButton->TabIndex = 3;
			this->abortButton->Text = L"Abort";
			this->abortButton->UseVisualStyleBackColor = true;
			this->abortButton->Click += gcnew System::EventHandler(this, &MainForm::abortButton_Click);
			// 
			// testButton
			// 
			this->testButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->testButton->Location = System::Drawing::Point(347, 513);
			this->testButton->Name = L"testButton";
			this->testButton->Size = System::Drawing::Size(75, 23);
			this->testButton->TabIndex = 4;
			this->testButton->Text = L"Benchmark";
			this->testButton->UseVisualStyleBackColor = true;
			this->testButton->Visible = false;
			this->testButton->Click += gcnew System::EventHandler(this, &MainForm::testButton_Click);
			// 
			// mainMenuStrip
			// 
			this->mainMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->fileToolStripMenuItem,
					this->viewToolStripMenuItem, this->helpToolStripMenuItem
			});
			this->mainMenuStrip->Location = System::Drawing::Point(0, 0);
			this->mainMenuStrip->Name = L"mainMenuStrip";
			this->mainMenuStrip->Size = System::Drawing::Size(584, 24);
			this->mainMenuStrip->TabIndex = 5;
			this->mainMenuStrip->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(7) {
				this->clearToolStripMenuItem,
					this->openToolStripMenuItem, this->toolStripSeparator, this->saveToolStripMenuItem, this->saveAsToolStripMenuItem, this->toolStripSeparator1,
					this->exitToolStripMenuItem
			});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(37, 20);
			this->fileToolStripMenuItem->Text = L"&File";
			// 
			// clearToolStripMenuItem
			// 
			this->clearToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"clearToolStripMenuItem.Image")));
			this->clearToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->clearToolStripMenuItem->Name = L"clearToolStripMenuItem";
			this->clearToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::N));
			this->clearToolStripMenuItem->Size = System::Drawing::Size(146, 22);
			this->clearToolStripMenuItem->Text = L"&Clear";
			this->clearToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::clearToolStripMenuItem_Click);
			// 
			// openToolStripMenuItem
			// 
			this->openToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"openToolStripMenuItem.Image")));
			this->openToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
			this->openToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::O));
			this->openToolStripMenuItem->Size = System::Drawing::Size(146, 22);
			this->openToolStripMenuItem->Text = L"&Open";
			this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::openToolStripMenuItem_Click);
			// 
			// toolStripSeparator
			// 
			this->toolStripSeparator->Name = L"toolStripSeparator";
			this->toolStripSeparator->Size = System::Drawing::Size(143, 6);
			// 
			// saveToolStripMenuItem
			// 
			this->saveToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"saveToolStripMenuItem.Image")));
			this->saveToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->saveToolStripMenuItem->Name = L"saveToolStripMenuItem";
			this->saveToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::S));
			this->saveToolStripMenuItem->Size = System::Drawing::Size(146, 22);
			this->saveToolStripMenuItem->Text = L"&Save";
			this->saveToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveToolStripMenuItem_Click);
			// 
			// saveAsToolStripMenuItem
			// 
			this->saveAsToolStripMenuItem->Name = L"saveAsToolStripMenuItem";
			this->saveAsToolStripMenuItem->Size = System::Drawing::Size(146, 22);
			this->saveAsToolStripMenuItem->Text = L"Save &As";
			this->saveAsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsToolStripMenuItem_Click);
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(143, 6);
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(146, 22);
			this->exitToolStripMenuItem->Text = L"E&xit";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exitToolStripMenuItem_Click);
			// 
			// viewToolStripMenuItem
			// 
			this->viewToolStripMenuItem->Name = L"viewToolStripMenuItem";
			this->viewToolStripMenuItem->Size = System::Drawing::Size(44, 20);
			this->viewToolStripMenuItem->Text = L"&View";
			// 
			// helpToolStripMenuItem
			// 
			this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->scripthelpToolStripMenuItem,
					this->checkUpdatesToolStripMenuItem, this->toolStripSeparator5, this->aboutToolStripMenuItem
			});
			this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
			this->helpToolStripMenuItem->Size = System::Drawing::Size(44, 20);
			this->helpToolStripMenuItem->Text = L"&Help";
			// 
			// scripthelpToolStripMenuItem
			// 
			this->scripthelpToolStripMenuItem->Name = L"scripthelpToolStripMenuItem";
			this->scripthelpToolStripMenuItem->Size = System::Drawing::Size(180, 22);
			this->scripthelpToolStripMenuItem->Text = L"&Help";
			this->scripthelpToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::scripthelpToolStripMenuItem_Click);
			// 
			// checkUpdatesToolStripMenuItem
			// 
			this->checkUpdatesToolStripMenuItem->Name = L"checkUpdatesToolStripMenuItem";
			this->checkUpdatesToolStripMenuItem->Size = System::Drawing::Size(180, 22);
			this->checkUpdatesToolStripMenuItem->Text = L"Check for &updates";
			this->checkUpdatesToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::checkUpdatesToolStripMenuItem_Click);
			// 
			// toolStripSeparator5
			// 
			this->toolStripSeparator5->Name = L"toolStripSeparator5";
			this->toolStripSeparator5->Size = System::Drawing::Size(177, 6);
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			this->aboutToolStripMenuItem->Size = System::Drawing::Size(180, 22);
			this->aboutToolStripMenuItem->Text = L"&About...";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::aboutToolStripMenuItem_Click);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(584, 561);
			this->Controls->Add(this->testButton);
			this->Controls->Add(this->abortButton);
			this->Controls->Add(this->processButton);
			this->Controls->Add(this->scriptText);
			this->Controls->Add(this->statusStrip);
			this->Controls->Add(this->mainMenuStrip);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MainMenuStrip = this->mainMenuStrip;
			this->Name = L"MainForm";
			this->Text = L"Bio Image Operation";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &MainForm::MainForm_FormClosing);
			this->Shown += gcnew System::EventHandler(this, &MainForm::MainForm_Shown);
			this->statusStrip->ResumeLayout(false);
			this->statusStrip->PerformLayout();
			this->mainMenuStrip->ResumeLayout(false);
			this->mainMenuStrip->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	};
}
