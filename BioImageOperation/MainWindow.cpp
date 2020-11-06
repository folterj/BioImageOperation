/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "MainWindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include "Keepalive.h"
#include "Util.h"

// https://mithatkonar.com/wiki/doku.php/qt/toward_robust_icon_support


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {

	ui.setupUi(this);
	connect(ui.actionNew, &QAction::triggered, this, &MainWindow::clearInput);
	connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::openDialog);
	connect(ui.actionSave, &QAction::triggered, this, &MainWindow::save);
	connect(ui.actionSave_As, &QAction::triggered, this, &MainWindow::saveDialog);
	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);

	connect(ui.scriptTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::textChanged);

	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);
	connect(ui.abortButton, &QAbstractButton::clicked, &scriptProcessing, &ScriptProcessing::doAbort);

	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].init(this, i);
	}

	for (int i = 0; i < Constants::nDisplays; i++) {
		textWindows[i].init(this, i);
	}

	scriptProcessing.registerObserver(this);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerElapsed()));
	timer->start(1s);
}

void MainWindow::setFilePath(string filepath) {
	this->filepath = filepath;
}

void MainWindow::updateTitle() {
	string title = "Bio Image Operation";
	string fileTitle = Util::extractTitle(filepath);
	if (fileTitle != "") {
		title += " - " + fileTitle;
		if (fileModified) {
			title += "*";
		}
	}
	setWindowTitle(Util::convertToQString(title));
}

void MainWindow::clearInput() {
	ui.scriptTextEdit->clear();
	fileModified = false;
}

void MainWindow::openDialog() {
	QString qfilename;
	string text = "";
	string line;

	try {
		qfilename = QFileDialog::getOpenFileName(this, tr("Load script"));
		if (qfilename != "") {
			filepath = qfilename.toStdString();
			scriptProcessing.doAbort();
			clearInput();
			ui.scriptTextEdit->setPlainText(Util::convertToQString(Util::readText(filepath)));
			fileModified = false;
			updateTitle();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e).c_str());
	}
}

void MainWindow::saveDialog() {
	QString qfilename;
	string extension;
	int extPos;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script"));
		if (qfilename != "") {
			filepath = qfilename.toStdString();
			save();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e).c_str());
	}
}

void MainWindow::save() {
	try {
		OutputStream output;
		output.init(filepath);
		output.write(ui.scriptTextEdit->toPlainText().toStdString());
		output.closeStream();

		if (fileModified) {
			fileModified = false;
			updateTitle();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e).c_str());
	}
}

void MainWindow::askSaveChanges() {
	if (fileModified) {
		if (QMessageBox::question(this, "File modified", "Save changes?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
			save();
		}
	}
}

void MainWindow::textChanged() {
	if (!fileModified) {
		fileModified = true;
		updateTitle();
	}
}

void MainWindow::process() {
	updateUI(true);
	scriptProcessing.startProcess(filepath, ui.scriptTextEdit->toPlainText().toStdString());
}

void MainWindow::updateUI(bool start) {
	ui.processButton->setEnabled(!start);
	ui.abortButton->setEnabled(start);
	ui.scriptTextEdit->setReadOnly(start);

	if (start) {
		Keepalive::startKeepAlive();
	} else {
		Keepalive::stopKeepAlive();
	}
}

void MainWindow::resetUI() {
	updateUI(false);
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

void MainWindow::showDialog(const char* message, MessageLevel level) {
	switch (level) {
	case MessageLevel::Error:
		QMessageBox::critical(this, "BIO", message);
		break;

	case MessageLevel::Warning:
		QMessageBox::warning(this, "BIO", message);
		break;

	default:
		QMessageBox::information(this, "BIO", message);
		break;
	}
}

void MainWindow::showText(const char* text, int displayi) {
	textWindows[displayi].showText(text);
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

	imageWindows[displayi].showImage(image);
	imageQueued = false;
}

void MainWindow::closeEvent(QCloseEvent* event) {
	askSaveChanges();
	// when main window is closed: close all child windows
	exit(0);
}
