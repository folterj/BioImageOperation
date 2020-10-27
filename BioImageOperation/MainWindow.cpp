/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "MainWindow.h"
#include "Util.h"

// https://mithatkonar.com/wiki/doku.php/qt/toward_robust_icon_support


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent) {

	ui.setupUi(this);
	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);
	connect(ui.abortButton, &QAbstractButton::clicked, &scriptProcessing, &ScriptProcessing::doAbort);

	imageWindow = new ImageWindow();

	scriptProcessing.registerObserver(this);
}

void MainWindow::setFilePath(string filepath) {
	this->filepath = filepath;
}

void MainWindow::process() {
	//updateUI(true);

	ui.scriptTextEdit->setPlainText(QString("OpenVideo('D:\\Video\\test.mkv')\n{\nShowImage()\n}"));

	scriptProcessing.startProcess(filepath, ui.scriptTextEdit->toPlainText().toStdString());
}

void MainWindow::resetUI() {

}

void MainWindow::resetImages() {

}

void MainWindow::resetProgressTimer() {
	time = Clock::now();
	processCount = 0;
	processFps = 0;
	statusQueued = false;
	imageQueued = false;
}

void MainWindow::clearStatus() {
	ui.statusBar->clearMessage();
	ui.progressBar->setValue(0);
}

bool MainWindow::checkStatusProcess() {
	bool ok = !statusQueued;
	processCount++;
	statusQueued = true;
	return ok;
}

void MainWindow::showStatus(const char* label, int i, int tot) {
	string s;
	double progress = 0;
	Clock::time_point now;
	Clock::duration totalElapsed;
	double totalElapseds;
	double avgFrametime;
	double estimateLeft = 0;

	now = Clock::now();
	totalElapsed = now - time;
	totalElapseds = (double)totalElapsed.count() / 1000000000;
	avgFrametime = totalElapseds / (i + 1);

	if (tot > 0) {
		progress = (double)i / tot;
		s += Util::format("%.2f", progress);
	}
	if (progress > 0) {
		estimateLeft = totalElapseds * (1 / progress - 1);
	}

	s += " " + string(label);
	s += Util::format(" (%.d)", i) + Util::format(" %.3fs", avgFrametime) + Util::format(" %.dfps", processFps);
	s += " Elapsed: " + Util::formatTimespan((int)totalElapseds);
	if (estimateLeft > 0) {
		s += " Left: " + Util::formatTimespan((int)estimateLeft);
	}

	ui.statusBar->showMessage(QString::fromUtf8(s.c_str()));
	ui.progressBar->setValue((int)(100 * progress));
	statusQueued = false;
}

void MainWindow::showInfo(const char* info, int displayi) {
	//infoForms[displayi]->showInfo(info);
}

void MainWindow::showError(const char* message) {
	//MessageBox::Show(message, "Operation error");
}

bool MainWindow::checkImageProcess() {
	bool ok = !imageQueued;
	imageQueued = true;
	return ok;
}

void MainWindow::showImage(Mat* image, int displayi) {
	imageWindow->draw(image);
	imageQueued = false;
}

void MainWindow::closeEvent(QCloseEvent* event) {
	exit(0);	// when main window is closed: close all child windows
}
