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
#include <QStyle>
#include "Keepalive.h"
#include "Util.h"
#include "config.h"


const QString DEFAULT_DIR_KEY("default_dir");

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {

	QFont font;
	int fontId = QFontDatabase::addApplicationFont(":/BioImageOperation/JetBrainsMono-Regular.ttf");
	if (fontId >= 0) {
		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
		font = QFont(family);
	} else {
		font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	}
	ui.setupUi(this);
	ui.scriptTextEdit->setFont(font);

	ui.actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui.actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui.actionSave_As->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui.actionCheck_for_Updates->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
	ui.actionHelp->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	ui.actionAbout->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));

	ui.processButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui.abortButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

	connect(ui.actionNew, &QAction::triggered, this, &MainWindow::clearInput);
	connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::openDialog);
	connect(ui.actionSave, &QAction::triggered, this, &MainWindow::save);
	connect(ui.actionSave_As, &QAction::triggered, this, &MainWindow::saveDialog);
	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui.actionCheck_for_Updates, &QAction::triggered, this, &MainWindow::checkUpdates);
	connect(ui.actionGenerate_help_doc, &QAction::triggered, this, &MainWindow::generateHelpDoc);
	connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
	connect(ui.actionAbout_Qt, &QAction::triggered, this, &MainWindow::showAboutQt);

	connect(ui.scriptTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::textChanged);

	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);
	connect(ui.abortButton, &QAbstractButton::clicked, &scriptProcessing, &ScriptProcessing::doAbort);

	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].init(this, i + 1);
	}

	for (int i = 0; i < Constants::nDisplays; i++) {
		textWindows[i].init(this, i + 1);
	}

	scriptProcessing.registerObserver(this);

	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &MainWindow::timerElapsed);
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
		qfilename = QFileDialog::getOpenFileName(this, tr("Load script"), bioSettings.value(DEFAULT_DIR_KEY).toString(), Util::convertToQString(Constants::scriptFileDialogFilter));
		if (qfilename != "") {
			bioSettings.setValue(DEFAULT_DIR_KEY, QVariant(qfilename));
			filepath = qfilename.toStdString();
			scriptProcessing.doAbort();
			clearInput();
			ui.scriptTextEdit->setPlainText(Util::convertToQString(Util::readText(filepath)));
			fileModified = false;
			updateTitle();
		}
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

void MainWindow::saveDialog() {
	QString qfilename;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script"), bioSettings.value(DEFAULT_DIR_KEY).toString(), Util::convertToQString(Constants::scriptFileDialogFilter));
		if (qfilename != "") {
			bioSettings.setValue(DEFAULT_DIR_KEY, QVariant(qfilename));
			filepath = qfilename.toStdString();
			save();
		}
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
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
	} catch (exception e) {
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
	for (int i = 0; i < Constants::nDisplays; i++) {
		textQueued[i] = false;
		imageQueued[i] = false;
	}
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
	double estimateLeft = 0;
	double avgFrametime;

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
			s += Util::format("%.1f%%", 100 * progress);
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
	QString title = tr("BIO");
	try {
		switch ((MessageLevel)level) {
		case MessageLevel::Error:
			QMessageBox::critical(this, title, Util::convertToQString(message));
			break;

		case MessageLevel::Warning:
			QMessageBox::warning(this, title, Util::convertToQString(message));
			break;

		default:
			QMessageBox::information(this, title, Util::convertToQString(message));
			break;
		}
	} catch (exception e) {
		cerr << Util::getExceptionDetail(e) << endl;
	}
}

bool MainWindow::checkTextProcess(int displayi) {
	bool ok = !textQueued[displayi];
	textQueued[displayi] = true;
	return ok;
}

void MainWindow::showText(string text, int displayi, string reference) {
	if (!textQueued[displayi]) {
		return;			// ignore queued events on abort
	}

	try {
		textWindows[displayi].showText(text, reference);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	textQueued[displayi] = false;
}

bool MainWindow::checkImageProcess(int displayi) {
	bool ok = !imageQueued[displayi];
	imageQueued[displayi] = true;
	return ok;
}

void MainWindow::showImage(Mat* image, int displayi, string reference) {
	if (!imageQueued[displayi]) {
		return;			// ignore queued events on abort
	}

	try {
		imageWindows[displayi].showImage(image, reference);
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	imageQueued[displayi] = false;
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

void MainWindow::showAboutQt() {
	QMessageBox::aboutQt(this);
}

void MainWindow::closeEvent(QCloseEvent* event) {
	// when main window is closed: ensure to close everything
	scriptProcessing.doAbort();
	askSaveChanges();
	QApplication::quit();
}
