/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "MainWindow.h"

// https://mithatkonar.com/wiki/doku.php/qt/toward_robust_icon_support


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent) {
	//initOpenCv("D:/Video/Ants/bio_ants.mp4");

	ui.setupUi(this);
	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);

	imageWindow = new ImageWindow();

	scriptProcessing.registerObserver(this);
}
/*
void MainWindow::initOpenCv(string filename) {
	videoCapture.open(filename);
}
*/

void MainWindow::setFilePath(string filePath) {
	this->filePath = filePath;
}

void MainWindow::resetUI() {

}

void MainWindow::resetImages() {

}

void MainWindow::resetProgressTimer() {

}

void MainWindow::clearStatus() {

}

void MainWindow::showStatus(int i) {

}

void MainWindow::showStatus(int i, int tot) {

}

void MainWindow::showStatus(string label, int i, int tot, bool showFrameProgress) {

}

void MainWindow::showStatus(string status, double progress) {

}

void MainWindow::showInfo(string info, int displayi) {

}

void MainWindow::displayImage(Mat* image, int displayi) {

}

void MainWindow::showErrorMessage(string message) {

}

void MainWindow::process() {
	//updateUI(true);
	scriptProcessing.startProcess(filePath, ui.scriptTextEdit->toPlainText().toStdString());
	/*
	videoCapture.grab();
	videoCapture.retrieve(videoFrame);
	//videoCapture >> videoFrame;
	imageWindow->draw(&videoFrame);
*/
}

void MainWindow::closeEvent(QCloseEvent* event) {
	exit(0);	// when main window is closed: close all child windows
}
