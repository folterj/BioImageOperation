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
#include "ui_MainWindow.h"
#include <opencv2/opencv.hpp>
#include "Observer.h"
#include "ImageWindow.h"
#include "ScriptProcessing.h"

using namespace std;
using namespace cv;


class MainWindow : public QMainWindow, public Observer
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	void process();
	void setFilePath(string filepath);
	virtual void resetUI() override;
	virtual void resetImages() override;
	virtual void resetProgressTimer() override;
	virtual void clearStatus() override;
	virtual void showStatus(int i) override;
	virtual void showStatus(int i, int tot) override;
	virtual void showStatus(string label, int i, int tot, bool showFrameProgress) override;
	virtual void showStatus(string status, double progress) override;
	virtual void showInfo(string info, int displayi) override;
	virtual void showErrorMessage(string message) override;

public slots:
	virtual void displayImage(Mat* image, int displayi) override;

private:
	string filepath;
	Ui::MainWindow ui;
	ImageWindow* imageWindow;
	ScriptProcessing scriptProcessing;

protected:
	void closeEvent(QCloseEvent* event);
};
