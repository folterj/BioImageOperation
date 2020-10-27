/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ImageWindow.h"
#include "Util.h"

// https://stackoverflow.com/questions/21246766/how-to-efficiently-display-opencv-video-in-qt
// https://amin-ahmadi.com/2018/03/29/how-to-read-process-and-display-videos-using-qt-and-opencv/

// https://stackoverflow.com/questions/24853687/qimage-and-threads
// https://wiki.qt.io/QThreads_general_usage
// https://www.qt.io/blog/2006/12/04/threading-without-the-headache
// https://stackoverflow.com/questions/4093159/what-is-the-correct-way-to-implement-a-qthread-example-please


ImageWindow::ImageWindow(QWidget *parent)
	: QMainWindow(parent) {
	ui.setupUi(this);

	ui.graphicsView->setScene(new QGraphicsScene(this));
	ui.graphicsView->scene()->addItem(&pixmap);
}

ImageWindow::~ImageWindow() {
}

void ImageWindow::draw(Mat* videoFrame) {
	if (isHidden()) {
		show();
	}

	if (videoFrame) {
		pixmap.setPixmap(QPixmap::fromImage(Util::matToQImage(*videoFrame)));
		pixmap.setTransformationMode(Qt::TransformationMode::SmoothTransformation);
		ui.graphicsView->fitInView(&pixmap, Qt::AspectRatioMode::KeepAspectRatio); // do this @ window resize only?
	}
}
