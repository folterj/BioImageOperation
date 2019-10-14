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

#include "MainForm.h"
#include "AboutForm.h"
#include "Keepalive.h"
#include "Util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace System::IO;
using namespace System::Reflection;
using namespace System::Windows::Forms;
using namespace BioImageOperation;


MainForm::MainForm(void)
{
	InitializeComponent();

	// delegates/invoke: https://stackoverflow.com/questions/229554/whats-the-difference-between-invoke-and-begininvoke

	secondsTimer.Interval = 1000;
	secondsTimer.Elapsed += gcnew System::Timers::ElapsedEventHandler(this, &MainForm::OnElapsed);
	secondsTimer.Start();

	for (int i = 0; i < imageForms->Length; i++)
	{
		imageForms[i] = gcnew ImageForm();
		imageForms[i]->Handle;				// Create window handle to allow form show from Invoke
		imageForms[i]->setTitle(i);
	}

	for (int i = 0; i < infoForms->Length; i++)
	{
		infoForms[i] = gcnew InfoForm();
		infoForms[i]->Handle;				// Create window handle to allow form show from Invoke
		infoForms[i]->setTitle(i);
	}

	operationInfoForm.Text = "Script operations";
	operationInfoForm.Width = 600;
	operationInfoForm.Height = 600;
}

MainForm::~MainForm()
{
	if (scriptProcessing)
	{
		delete scriptProcessing;
	}

	if (benchMark)
	{
		delete benchMark;
	}

	if (imageForms)
	{
		for (int i = 0; i < imageForms->Length; i++)
		{
			if (imageForms[i])
			{
				delete imageForms[i];
			}
		}
		delete[] imageForms;
	}

	if (infoForms)
	{
		for (int i = 0; i < infoForms->Length; i++)
		{
			if (infoForms[i])
			{
				delete infoForms[i];
			}
		}
		delete[] infoForms;
	}

	if (components)
	{
		delete components;
	}
}

System::Void MainForm::MainForm_Shown(System::Object^  sender, System::EventArgs^  e)
{
	scriptText->AutoWordSelection = false;	// work-around to RTB selection bug
}

void MainForm::setFilePath(System::String^ filePath)
{
	System::String^ title = "Bio Image Operation";
	System::String^ fileTitle;

	this->filePath = filePath;
	fileTitle = Util::extractTitle(filePath);
	if (fileTitle != "")
	{
		title += " - " + fileTitle;
	}
	Text = title;
}

void MainForm::clearInput()
{
	scriptText->Clear();
}

void MainForm::open()
{
	StreamReader reader(filePath);

	scriptProcessing->doAbort(false);
	clearInput();

	scriptText->AppendText(reader.ReadToEnd());

	reader.Close();
}

void MainForm::save()
{
	StreamWriter writer(filePath);

	writer.Write(scriptText->Text);

	writer.Close();
}

void MainForm::updateUI(bool start)
{
	processButton->Enabled = !start;
	testButton->Enabled = !start;
	abortButton->Enabled = start;
	scriptText->ReadOnly = start;

	if (start)
	{
		Keepalive::startKeepAlive();
	}
	else
	{
		Keepalive::stopKeepAlive();
	}
}

void MainForm::resetUI()
{
	this->Invoke(resetUIDelegate);
}

void MainForm::resetUIDelegateMethod()
{
	updateUI(false);
}

void MainForm::resetImages()
{
	for (int i = 0; i < imageForms->Length; i++)
	{
		imageForms[i]->reset();
	}
}

void MainForm::resetProgressTimer()
{
	stopwatch.Restart();
	processCount = 0;
	processFps = 0;
}

void MainForm::clearStatus()
{
	showStatus("", 0);
}

void MainForm::showStatus(int i)
{
	System::String^ s;

	TimeSpan totalElapsed = stopwatch.Elapsed;
	double totalTimeElapsed = totalElapsed.TotalSeconds;
	double avgFrametime = totalTimeElapsed / (i + 1);

	s = System::String::Format("#{0} {1:F3}s @{2}fps Elapsed: {3:hh\\:mm\\:ss}", i, avgFrametime, processFps, totalElapsed);

	showStatus(s, 0);

	processCount++;
}

void MainForm::showStatus(int i, int tot)
{
	showStatus("", i, tot, false);
}

void MainForm::showStatus(System::String^ label, int i, int tot, bool showFrameProgress)
{
	System::String^ s;

	double progress = (double)i / tot;
	TimeSpan totalElapsed = stopwatch.Elapsed;
	double totalTimeElapsed = totalElapsed.TotalSeconds;
	double avgFrametime = totalTimeElapsed / (i + 1);
	double estimate = 0;
	TimeSpan estimateTimespan;

	if (progress > 0)
	{
		estimate = totalTimeElapsed * (1 / progress - 1);
	}
	estimateTimespan = TimeSpan::FromSeconds(estimate);

	if (showFrameProgress)
	{
		s = System::String::Format("{0:P1} {1} (#{2}) {3:F3}s @{4}fps Elapsed: {5:hh\\:mm\\:ss} Left: {6:hh\\:mm\\:ss}", progress, label, i, avgFrametime, processFps, totalElapsed, estimateTimespan);
	}
	else
	{
		s = System::String::Format("{0:P1} {1} Elapsed: {2:hh\\:mm\\:ss} Left: {3:hh\\:mm\\:ss}", progress, label, totalElapsed, estimateTimespan);
	}

	showStatus(s, progress);

	processCount++;
}

void MainForm::showStatus(System::String^ status, double progress)
{
	statusStrip->BeginInvoke(showStatusDelegate, gcnew array<Object^>{status, progress});
}

void MainForm::showStatusDelegateMethod(System::String^ status, double progress)
{
	statusLabel->Text = status;
	if (progress > 1)
	{
		progress = 1;
	}
	progressBar->Value = (int)(100 * progress);
}

void MainForm::displayImage(Mat* image, int displayi)
{
	if (displayi < 0 || displayi >= imageForms->Length)
	{
		displayi = 0;
	}

	if (imageForms[displayi]->setImage(image))
	{
		this->BeginInvoke(displayImageDelegate, gcnew array<Object^>{displayi});	// invoke async on control
	}	
}

void MainForm::displayImageDelegateMethod(int displayi)
{
	imageForms[displayi]->show();
}

void MainForm::showInfo(System::String^ info, int displayi)
{
	if (displayi < 0 || displayi >= imageForms->Length)
	{
		displayi = 0;
	}

	this->BeginInvoke(showInfoDelegate, gcnew array<Object^>{info, displayi});
}

void MainForm::showInfoDelegateMethod(System::String^ info, int displayi)
{
	infoForms[displayi]->showInfo(info);
}

void MainForm::showErrorMessage(System::String^ message)
{
	this->Invoke(showErrorMessageDelegate, gcnew array<Object^>{message});
}

void MainForm::showErrorMessageDelegateMethod(System::String^ message)
{
	MessageBox::Show(message, "Operation error");
}

void MainForm::OnElapsed(System::Object ^sender, System::Timers::ElapsedEventArgs ^e)
{
	if (processCount != 0)
	{
		processFps = processCount;
		processCount = 0;
	}

	for (int i = 0; i < imageForms->Length; i++)
	{
		imageForms[i]->updateFps();
	}
}

System::Void MainForm::clearToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	setFilePath("");
	clearInput();
}

System::Void MainForm::openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	OpenFileDialog fileDialog;

	fileDialog.Filter = Util::netString(Constants::scriptFileDialogFilter);
	fileDialog.FilterIndex = Constants::defaultScriptFileDialogFilter;

	if (fileDialog.ShowDialog() == ::DialogResult::OK)
	{
		try
		{
			setFilePath(fileDialog.FileName);
			open();
		}
		catch (InvalidOperationException^ ex)
		{
			MessageBox::Show(ex->Message);
		}
	}
}

System::Void MainForm::saveToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	if (filePath != "")
	{
		save();
	}
	else
	{
		saveAsToolStripMenuItem_Click(sender, e);
	}
}

System::Void MainForm::saveAsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	SaveFileDialog fileDialog;

	fileDialog.Filter = Util::netString(Constants::scriptFileDialogFilter);
	fileDialog.FilterIndex = Constants::defaultScriptFileDialogFilter;
	if (filePath != "")
	{
		fileDialog.FileName = Util::extractFileName(filePath);
	}
	if (fileDialog.ShowDialog() == ::DialogResult::OK)
	{
		setFilePath(fileDialog.FileName);
		save();
	}
}

System::Void MainForm::exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	Close();
}

System::Void MainForm::scripthelpToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	operationInfoForm.showRtf(ScriptOperation::getOperationList());
}

System::Void MainForm::aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	AboutForm about;
	about.ShowDialog();
}

System::Void MainForm::checkUpdatesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	Thread^ updateThread = gcnew Thread(gcnew ThreadStart(this, &MainForm::checkUpdates));
	updateThread->Start();
}

void MainForm::checkUpdates()
{
	Assembly^ assembly;
	System::String^ currentVersion;
	System::String^ webVersion;

	try
	{
		assembly = Assembly::GetExecutingAssembly();
		currentVersion = assembly->GetName()->Version->ToString();
		webVersion = Util::getUrl(Util::netString(Constants::webFilesUrl) + "BIOver");
		if (webVersion != "")
		{
			if (Util::compareVersions(currentVersion, webVersion) > 0)
			{
				// newer version found
				if (MessageBox::Show("New version available for download\nGo to web page now?", "BIO Update",
					MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes)
				{
					Util::openWebLink(Util::netString(Constants::webPage));
				}
				return;
			}
			else
			{
				// same (or older) version
				MessageBox::Show("Current version is up to date.", "BIO Update");
				return;
			}
		}
	}
	catch (System::Exception^)
	{
	}

	// unable to check version
	MessageBox::Show("No newer version found.", "BIO Update");
}

System::Void MainForm::testButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	updateUI(true);
	benchMark->doTest();
}

System::Void MainForm::processButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	updateUI(true);
	scriptProcessing->startProcess(filePath, scriptText->Text);
}

System::Void MainForm::abortButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	scriptProcessing->doAbort(true);
	benchMark->doAbort();
}

System::Void MainForm::MainForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	secondsTimer.Stop();
	scriptProcessing->doAbort(true);
	benchMark->doAbort();
}
