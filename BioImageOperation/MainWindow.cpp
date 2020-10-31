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


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {

	ui.setupUi(this);
	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);
	connect(ui.abortButton, &QAbstractButton::clicked, &scriptProcessing, &ScriptProcessing::doAbort);

	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].init(this, i);
	}

	scriptProcessing.registerObserver(this);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerElapsed()));
	timer->start(1s);
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
	/*
	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].reset();
	}
	*/
}

void MainWindow::resetProgressTimer() {
	time = Clock::now();
	processCount = 0;
	processFps = 0;
	statusQueued = false;
	imageQueued = false;
}

void MainWindow::timerElapsed() {
	if (processCount != 0) {
		processFps = processCount;
		processCount = 0;
	}

	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].updateFps();
	}
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

void MainWindow::showStatus(int i, int tot, const char* label) {
	string s;
	double progress = 0;
	Clock::time_point now;
	chrono::duration<double> totalElapsed;
	double totalElapseds;
	double avgFrametime;
	double estimateLeft = 0;

	if (!statusQueued) {
		return;			// ignore queued events on abort
	}

	now = Clock::now();
	totalElapsed = now - time;
	totalElapseds = totalElapsed.count();
	avgFrametime = totalElapseds / (i + 1);

	if (tot > 0) {
		progress = (double)i / tot;
		s += Util::format("%.2f", progress);
	}
	if (progress > 0) {
		estimateLeft = totalElapseds * (1 / progress - 1);
	}

	s += Util::format(" %s (#%d) %.3fs @%dfps", label, i, avgFrametime, processFps);
	s += " Elapsed: " + Util::formatTimespan((int)totalElapseds);
	if (estimateLeft > 0) {
		s += " Left: " + Util::formatTimespan((int)estimateLeft);
	}

	ui.statusBar->showMessage(Util::convertToQString(s));
	ui.progressBar->setValue((int)(100 * progress));
	statusQueued = false;
}

void MainWindow::showInfo(const char* info, int displayi) {
	//infoForms[displayi]->showInfo(info);
}

bool MainWindow::checkImageProcess() {
	bool ok = !imageQueued;
	imageQueued = true;
	return ok;
}

void MainWindow::showImage(Mat* image, int displayi) {
	if (!imageQueued) {
		return;			// ignore queued events on abort
	}

	if (displayi < 0 || displayi >= Constants::nDisplays) {
		displayi = 0;
	}

	//if (imageWindows[displayi].setImage(image)) {
		imageWindows[displayi].draw(image);
	//}
	imageQueued = false;
}

void MainWindow::showDialog(const char* message) {
	//MessageBox::Show(message, "Operation error");
}

void MainWindow::closeEvent(QCloseEvent* event) {
	exit(0);	// when main window is closed: close all child windows
}
