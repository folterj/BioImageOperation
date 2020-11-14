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
#include <QGraphicsPixmapItem>
#include "ui_ImageWindow.h"
#include <opencv2/opencv.hpp>
#include "Observer.h"

using namespace cv;


class ImageWindow : public QMainWindow
{
	Q_OBJECT

private:
	Ui::ImageWindow ui;
	QGraphicsPixmapItem pixmap;
	Observer* observer;
	Mat* image;
	int title;
	int swidth = 0;
	int sheight = 0;
	int displayCount = 0;
	int displayFps = 0;

public:
	ImageWindow(QWidget* parent = Q_NULLPTR);
	~ImageWindow();
	void init(Observer* observer, int title);
	void updateFps();
	void updateTitle();
	void showImage(Mat* image);
	void saveImage();
};
