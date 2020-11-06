/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageWindow.h"
#include <QFileDialog>
#include "Constants.h"
#include "Util.h"

// https://stackoverflow.com/questions/21246766/how-to-efficiently-display-opencv-video-in-qt
// https://amin-ahmadi.com/2018/03/29/how-to-read-process-and-display-videos-using-qt-and-opencv/

// https://stackoverflow.com/questions/24853687/qimage-and-threads
// https://wiki.qt.io/QThreads_general_usage
// https://www.qt.io/blog/2006/12/04/threading-without-the-headache
// https://stackoverflow.com/questions/4093159/what-is-the-correct-way-to-implement-a-qthread-example-please


ImageWindow::ImageWindow(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);

	ui.graphicsView->setScene(new QGraphicsScene(this));
	ui.graphicsView->scene()->addItem(&pixmap);
}

ImageWindow::~ImageWindow() {
}

void ImageWindow::init(Observer* observer, int title) {
	this->observer = observer;
	this->title = title;
	updateTitle();
}

void ImageWindow::updateFps() {
	displayFps = displayCount;
	displayCount = 0;
}

void ImageWindow::updateTitle() {
	string s = Util::format("BIO Image %d", title);

	if (swidth != 0 && sheight != 0) {
		s += Util::format(" %dx%d", swidth, sheight);
	}
	if (displayFps != 0) {
		s += Util::format(" @%dfps", displayFps);
	}
	setWindowTitle(Util::convertToQString(s));
}

void ImageWindow::showImage(Mat* image) {
	if (isHidden()) {
		show();
	}

	swidth = image->cols;
	sheight = image->rows;
	this->image = image;

	pixmap.setPixmap(QPixmap::fromImage(Util::matToQImage(*image)));
	pixmap.setTransformationMode(Qt::TransformationMode::SmoothTransformation);
	ui.graphicsView->fitInView(&pixmap, Qt::AspectRatioMode::KeepAspectRatio); // do this @ window resize only?
	ui.graphicsView->repaint();

	displayCount++;
	updateTitle();
}

void ImageWindow::saveImage() {
	QString qfilename;
	string filename;
	string extension;
	int extPos;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save Image"));
		if (qfilename != "") {
			filename = qfilename.toStdString();
			extPos = filename.rfind(".");
			if (extPos < 0) {
				extension = Constants::defaultImageExtension;
				if (!extension._Starts_with(".")) {
					extension = "." + extension;
				}
				filename += extension;
			}
			Util::saveImage(filename, image);
		}
	} catch (cv::Exception e) {
		// opencv exception
		if (e.err != "") {
			observer->showDialog(e.err.c_str());
		} else {
			observer->showDialog(e.msg.c_str());
		}
	} catch (std::exception e) {
		observer->showDialog(Util::getExceptionDetail(e).c_str());
	}
}
