/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include "ui_MainWindow.h"
#include <opencv2/opencv.hpp>
#include "Observer.h"
#include "ImageWindow.h"
#include "TextWindow.h"
#include "AboutWindow.h"
#include "ScriptProcessing.h"
#include "Types.h"

using namespace std;
using namespace cv;


class MainWindow : public QMainWindow, public Observer
{
	Q_OBJECT

private:
	QSettings bioSettings;
	Ui::MainWindow ui;
	ImageWindow imageWindows[4];
	TextWindow textWindows[4];
	TextWindow scriptHelpWindow;
	AboutWindow aboutWindow;
	ScriptProcessing scriptProcessing;
	QTimer* timer;
	Clock::time_point time;
	string filepath;
	bool fileModified = false;
	bool statusQueued = false;
	bool textQueued[4] = { false, false, false, false };
	bool imageQueued[4] = { false, false, false, false };
	int processCount = 0;
	int processFps = 0;

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	void setFilePath(string filepath);
	void updateTitle();
	void clearInput();
	void openDialog();
	bool saveDialog();
	bool save();
	bool askSaveChanges();
	void textChanged();
	void process();
	void updateUI(bool start);
	void timerElapsed();
	virtual void resetProgressTimer() override;
	virtual bool checkStatusProcess() override;
	virtual bool checkTextProcess(int displayi) override;
	virtual bool checkImageProcess(int displayi) override;

signals:
	void resetUI();
	void clearStatus();
	void showStatus(int i, int tot = 0, string label = "");
	void showDialog(string message, int level = (int)MessageLevel::Info);
	void showText(string text, int displayi, string reference = "");
	void showImage(Mat* image, int displayi, string reference = "");

public slots:
	void resetUIQt();
	void clearStatusQt();
	void showStatusQt(int i, int tot = 0, string label = "");
	void showDialogQt(string message, int level = (int)MessageLevel::Info);
	void showTextQt(string text, int displayi, string reference = "");
	void showImageQt(Mat* image, int displayi, string reference = "");

protected:
	void checkUpdates();
	void showScriptHelp();
	void saveScriptHelp();
	void showAbout();
	void showAboutQt();
	void closeEvent(QCloseEvent* event);
};
