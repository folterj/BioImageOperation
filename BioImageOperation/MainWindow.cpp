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
#include "config.h"

// https://mithatkonar.com/wiki/doku.php/qt/toward_robust_icon_support


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {

	ui.setupUi(this);
	connect(ui.actionNew, &QAction::triggered, this, &MainWindow::clearInput);
	connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::openDialog);
	connect(ui.actionSave, &QAction::triggered, this, &MainWindow::save);
	connect(ui.actionSave_As, &QAction::triggered, this, &MainWindow::saveDialog);
	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui.actionCheck_for_Updates, &QAction::triggered, this, &MainWindow::checkUpdates);
	connect(ui.actionGenerate_help_doc, &QAction::triggered, this, &MainWindow::generateHelpDoc);
	connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::showAbout);

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
	updateTitle();
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

	try {
		qfilename = QFileDialog::getOpenFileName(this, tr("Load script"), QString(), Util::convertToQString(Constants::scriptFileDialogFilter));
		if (qfilename != "") {
			filepath = qfilename.toStdString();
			scriptProcessing.doAbort();
			clearInput();
			ui.scriptTextEdit->setPlainText(Util::convertToQString(Util::readText(filepath)));
			fileModified = false;
			updateTitle();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e));
	}
}

void MainWindow::saveDialog() {
	QString qfilename;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script"), QString(), Util::convertToQString(Constants::scriptFileDialogFilter));
		if (qfilename != "") {
			filepath = qfilename.toStdString();
			save();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e));
	}
}

void MainWindow::save() {
	try {
		OutputStream output(filepath);
		output.write(ui.scriptTextEdit->toPlainText().toStdString());
		output.closeStream();

		if (fileModified) {
			fileModified = false;
			updateTitle();
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e));
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
	try {
		updateUI(false);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

void MainWindow::resetProgressTimer() {
	time = Clock::now();
	processCount = 0;
	processFps = 0;
	statusQueued = false;
	textQueued = false;
	imageQueued = false;
}

void MainWindow::timerElapsed() {
	if (processCount != 0) {
		processFps = processCount;
		processCount = 0;
	}

	try {
		for (int i = 0; i < Constants::nDisplays; i++) {
			imageWindows[i].updateFps();
		}
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

void MainWindow::clearStatus() {
	try {
		ui.statusBar->clearMessage();
		ui.progressBar->setValue(0);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

bool MainWindow::checkStatusProcess() {
	bool ok = !statusQueued;
	processCount++;
	statusQueued = true;
	return ok;
}

void MainWindow::showStatus(int i, int tot, string label) {
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

	try {
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

		s += Util::format(" %s (#%d) %.3fs @%dfps", label.c_str(), i, avgFrametime, processFps);
		s += " Elapsed: " + Util::formatTimespan((int)totalElapseds);
		if (estimateLeft > 0) {
			s += " Left: " + Util::formatTimespan((int)estimateLeft);
		}

		ui.statusBar->showMessage(Util::convertToQString(s));
		ui.progressBar->setValue((int)(100 * progress));
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	statusQueued = false;
}

void MainWindow::showDialog(string message, int level) {
	try {
		switch ((MessageLevel)level) {
		case MessageLevel::Error:
			QMessageBox::critical(this, tr("BIO"), Util::convertToQString(message));
			break;

		case MessageLevel::Warning:
			QMessageBox::warning(this, tr("BIO"), Util::convertToQString(message));
			break;

		default:
			QMessageBox::information(this, tr("BIO"), Util::convertToQString(message));
			break;
		}
	} catch (exception e) {
		cerr << Util::getExceptionDetail(e) << endl;
	}
}

bool MainWindow::checkTextProcess() {
	bool ok = !textQueued;
	textQueued = true;
	return ok;
}

void MainWindow::showText(string text, int displayi) {
	if (!textQueued) {
		return;			// ignore queued events on abort
	}

	try {
		textWindows[displayi].showText(text);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	textQueued = false;
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

	try {
		if (displayi < 0 || displayi >= Constants::nDisplays) {
			displayi = 0;
		}
		imageWindows[displayi].showImage(image);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	imageQueued = false;
}

void MainWindow::checkUpdates() {
	string currentVersion, webVersion;
	try {
		currentVersion = PROJECT_VER;
		webVersion = Util::getUrl(Constants::webFilesUrl + "BIOver");
		if (webVersion != "") {
			if (Util::compareVersions(currentVersion, webVersion) > 0) {
				// newer version found
				if (QMessageBox::question(this, "BIO", "New version available for download\nGo to web page now?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
					Util::openWebLink(Constants::webPage);
				}
				return;
			} else {
				// same (or older) version
				showDialog("Current version is up to date.");
				return;
			}
		} else {
			showDialog("Unable to check online version information.");
		}
	} catch (...) { }
}

void MainWindow::generateHelpDoc() {
	QString qfilename;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script"), QString(), Util::convertToQString(Constants::helpDocDialogFilter));
		if (qfilename != "") {
			ScriptOperation::writeOperationList(qfilename.toStdString());
		}
	} catch (std::exception e) {
		showDialog(Util::getExceptionDetail(e));
	}	
}

void MainWindow::showAbout() {
	aboutWindow.exec();
}

void MainWindow::closeEvent(QCloseEvent* event) {
	askSaveChanges();
	// when main window is closed: close all child windows
	exit(0);
}