/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <chrono>
#include <QMainWindow>
#include "ui_MainWindow.h"
#include <opencv2/opencv.hpp>
#include "Observer.h"
#include "ImageWindow.h"
#include "ScriptProcessing.h"

using namespace std;
using namespace cv;

typedef std::chrono::high_resolution_clock Clock;


class MainWindow : public QMainWindow, public Observer
{
	Q_OBJECT

private:
	string filepath;
	Ui::MainWindow ui;
	ImageWindow* imageWindow;
	ScriptProcessing scriptProcessing;
	Clock::time_point time;
	bool statusQueued = false;
	bool imageQueued = false;
	int processCount = 0;
	int processFps = 0;

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	void setFilePath(string filepath);
	void process();
	virtual void resetProgressTimer() override;
	virtual bool checkStatusProcess() override;
	virtual bool checkImageProcess() override;

public slots:
	virtual void resetUI() override;
	virtual void resetImages() override;
	virtual void clearStatus() override;
	virtual void showStatus(const char* label, int i, int tot) override;
	virtual void showInfo(const char* info, int displayi) override;
	virtual void showError(const char* message) override;
	virtual void showImage(Mat* image, int displayi) override;

protected:
	void closeEvent(QCloseEvent* event);
};
