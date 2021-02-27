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
#include <QTextStream>
#include "Util.h"
#include "Constants.h"
#include "config.h"


const QString DEFAULT_DIR_KEY("default_dir");

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent) {

	QFont font;
	int fontId = QFontDatabase::addApplicationFont(":/BioImageOperation/JetBrainsMono.ttf");
	if (fontId >= 0) {
		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
		font = QFont(family);
	} else {
		font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	}
	font.setPointSize(9);

	ui.setupUi(this);
	ui.scriptTextEdit->setFont(font);
	ui.overlayTextEdit->setFont(font);
	debugWindow.setFont(font);

	ui.actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui.actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui.actionSaveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui.actionCheckForUpdates->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
	ui.actionScriptHelp->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	ui.actionSaveScriptHelp->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui.actionHelp->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	ui.actionAbout->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));

	defaultProcessText = ui.processButton->text().toStdString();
	ui.processButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui.abortButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

	connect(ui.actionClear, &QAction::triggered, this, &MainWindow::clearDialog);
	connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::openDialog);
	connect(ui.actionSave, &QAction::triggered, this, &MainWindow::save);
	connect(ui.actionSaveAs, &QAction::triggered, this, &MainWindow::saveDialog);
	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);

	connect(ui.actionGenerateTrackingScript, &QAction::triggered, this, &MainWindow::generateTrackingScript);

	connect(ui.actionScriptHelp, &QAction::triggered, this, &MainWindow::showScriptHelp);
	connect(ui.actionSaveScriptHelp, &QAction::triggered, this, &MainWindow::saveScriptHelp);
	connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
	connect(ui.actionCheckForUpdates, &QAction::triggered, this, &MainWindow::checkUpdates);
	connect(ui.actionAboutQt, &QAction::triggered, this, &MainWindow::showAboutQt);

	connect(ui.scriptTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::textChanged);

	connect(ui.processButton, &QAbstractButton::clicked, this, &MainWindow::process);
	connect(ui.abortButton, &QAbstractButton::clicked, &scriptProcessing, &ScriptProcessing::requestAbort);

	connect(this, &MainWindow::setMode, this, &MainWindow::setModeQt);
	connect(this, &MainWindow::clearStatus, this, &MainWindow::clearStatusQt);
	connect(this, &MainWindow::showStatus, this, &MainWindow::showStatusQt);
	connect(this, &MainWindow::showText, this, &MainWindow::showTextQt);
	connect(this, &MainWindow::showImage, this, &MainWindow::showImageQt);
	connect(this, &MainWindow::showDialog, this, &MainWindow::showDialogQt);
	connect(this, &MainWindow::showOperations, this, &MainWindow::showOperationsQt);

	operationHighlighter = new QOperationHighlighter(ui.overlayTextEdit->document());

	for (int i = 0; i < Constants::nDisplays; i++) {
		imageWindows[i].init(this, i + 1);
	}

	for (int i = 0; i < Constants::nDisplays; i++) {
		textWindows[i].init(this, i + 1);
	}

	scriptHelpWindow.init(this);
	debugWindow.init(this);

	scriptProcessing.registerObserver(this);

	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &MainWindow::timerElapsed);
	timer->start(1s);
}

void MainWindow::setFilePath(string filepath) {
	this->filepath = filepath;
	updateTitle();
}

void MainWindow::setText(string text) {
	ignoreTextChangeEvent = true;
	ui.scriptTextEdit->setPlainText(Util::convertToQString(text));
	ignoreTextChangeEvent = false;
}

void MainWindow::updateTitle() {
	string title = "Bio Image Operation";
	string fileTitle = Util::extractTitle(filepath);
	if (fileTitle != "") {
		title += " - " + fileTitle;
	}
	if (fileModified) {
		title += "*";
	}
	setWindowTitle(Util::convertToQString(title));
}

void MainWindow::clearInput() {
	ui.scriptTextEdit->clear();
	fileModified = false;
	setFilePath("");
}

bool MainWindow::clearDialog() {
	if (askSaveChanges()) {
		clearInput();
		return true;
	}
	return false;
}

bool MainWindow::openDialog() {
	QString qfilename;

	if (askSaveChanges()) {
		try {
			qfilename = QFileDialog::getOpenFileName(this, tr("Load script"), bioSettings.value(DEFAULT_DIR_KEY).toString(), Util::convertToQString(Constants::scriptFileDialogFilter));
			if (qfilename != "") {
				clearInput();
				bioSettings.setValue(DEFAULT_DIR_KEY, QVariant(qfilename));
				setFilePath(qfilename.toStdString());
				setText(Util::readText(filepath));
				scriptProcessing.doReset();
				return true;
			}
		} catch (exception e) {
			showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
		}
	}
	return false;
}

bool MainWindow::saveDialog() {
	QString qfilename;

	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script"), bioSettings.value(DEFAULT_DIR_KEY).toString(), Util::convertToQString(Constants::scriptFileDialogFilter));
		if (qfilename != "") {
			bioSettings.setValue(DEFAULT_DIR_KEY, QVariant(qfilename));
			filepath = qfilename.toStdString();
			return save();
		}
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
	return false;
}

bool MainWindow::save() {
	if (filepath == "") {
		return saveDialog();
	} else {
		try {
			OutputStream output(filepath);
			output.write(ui.scriptTextEdit->toPlainText().toStdString());
			output.closeStream();
			fileModified = false;
			updateTitle();
		} catch (exception e) {
			showDialog(Util::getExceptionDetail(e));
			return false;
		}
	}
	return true;
}

bool MainWindow::askSaveChanges() {
	QMessageBox::StandardButton response;
	if (fileModified) {
		response = QMessageBox::question(this, "File modified", "Save changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (response == QMessageBox::Yes) {
			return save();
		}
		if (response == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}

bool MainWindow::askCloseInProgress() {
	QMessageBox::StandardButton response;
	if (scriptProcessing.getMode() != OperationMode::Idle) {
		response = QMessageBox::question(this, "Operation in progress", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No);
		if (response == QMessageBox::No) {
			return false;
		}
	}
	return true;
}

void MainWindow::generateTrackingScript() {
	generateScript("Tracking", "Select source video", Constants::trackingScriptTemplate);
}

void MainWindow::generateScript(string title, string instruction, string scriptFilename) {
	QString qfilename;
	string script;

	if (askSaveChanges()) {
		try {
			QFile file(Util::convertToQString(scriptFilename));
			if (file.open(QFile::ReadOnly)) {
				QTextStream stream(&file);
				script = stream.readAll().toStdString();
			}
			qfilename = QFileDialog::getOpenFileName(this, Util::convertToQString(instruction));
			if (qfilename != "") {
				clearInput();
				filepath = qfilename.toStdString();
				script = Util::replace(script, Constants::filenameTemplate, Util::extractFileName(filepath));
				fileModified = true;
				setFilePath(Util::combinePath(Util::extractFilePath(filepath), title + " " + Util::extractTitle(filepath) + "." + Constants::defaultScriptExtension));
				bioSettings.setValue(DEFAULT_DIR_KEY, QVariant(Util::convertToQString(filepath)));
				setText(script);
				scriptProcessing.doReset();
			}
		}
		catch (exception e) {
			showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
		}
	}
}

void MainWindow::textChanged() {
	if (!ignoreTextChangeEvent) {
		// simplified logic
		if (fileModified == ui.scriptTextEdit->toPlainText().isEmpty()) {
			fileModified = !fileModified;
			updateTitle();
		}
	}
}

void MainWindow::process() {
	scriptProcessing.startProcess(filepath, ui.scriptTextEdit->toPlainText().toStdString());
}

void MainWindow::requestPause() {
	scriptProcessing.requestPause();
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

void MainWindow::setModeQt(int mode0) {
	OperationMode mode = (OperationMode)mode0;
	bool controlsEnabled = (mode == OperationMode::Idle);
	string buttonText;

	try {
		if (mode == OperationMode::Run) {
			buttonText = "Pause";
			ui.processButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		} else if (mode == OperationMode::Pause) {
			buttonText = "Continue";
			ui.processButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		} else if (mode == OperationMode::Abort) {
			buttonText = "Aborting...";
		} else {
			buttonText = defaultProcessText;
			ui.processButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		}
		ui.processButton->setText(Util::convertToQString(buttonText));
		ui.processButton->setEnabled(mode != OperationMode::Abort);
		ui.abortButton->setEnabled(!controlsEnabled);
		ui.actionClear->setEnabled(controlsEnabled);
		ui.actionOpen->setEnabled(controlsEnabled);
		if (controlsEnabled) {
			ui.stackedWidget->setCurrentWidget(ui.scriptTextEdit);
		} else {
			ui.stackedWidget->setCurrentWidget(ui.overlayTextEdit);
		}
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

void MainWindow::clearStatusQt() {
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

void MainWindow::showStatusQt(int i, int tot, string label) {
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

bool MainWindow::checkOperationsProcess() {
	bool ok = !operationQueued;
	operationQueued = true;
	return ok;
}

void MainWindow::showOperationsQt(ScriptOperations* operations, ScriptOperation* currentOperation) {
	string text;
	operationHighlighter->setOperations(operations, currentOperation);
	vector<string> lines = Util::split(ui.scriptTextEdit->toPlainText().toStdString(), "\n");
	operations->renderText(&lines);
	for (string line : lines) {
		text += line + "\n";
	}
	ui.overlayTextEdit->setPlainText(Util::convertToQString(text));
	operationQueued = false;
}

void MainWindow::showDialogQt(string message, int level) {
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

void MainWindow::showTextQt(string text, int displayi, string reference) {
	if (!textQueued[displayi]) {
		return;			// ignore queued events on abort
	}

	try {
		if (displayi >= Constants::nTextWindows) {
			debugWindow.showText(text, reference);
		} else {
			textWindows[displayi].showText(text, reference);
		}
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

void MainWindow::showImageQt(Mat* image, int displayi, string reference) {
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

void MainWindow::showScriptHelp() {
	scriptHelpWindow.showText(ScriptOperation::getOperationList(false));
}

void MainWindow::saveScriptHelp() {
	QString qfilename;
	try {
		qfilename = QFileDialog::getSaveFileName(this, tr("Save script help"), Util::convertToQString(Constants::scriptHelpDefaultFilename), Util::convertToQString(Constants::scriptHelpDialogFilter));
		if (qfilename != "") {
			OutputStream outputStream(qfilename.toStdString());
			outputStream.write(ScriptOperation::getOperationList(true));
			outputStream.closeStream();
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
	if (askSaveChanges()) {
		if (askCloseInProgress()) {
			scriptProcessing.requestAbort();
			this_thread::sleep_for(100ms);		// finish async tasks (show image)
			QApplication::quit();
			return;
		}
	}
	event->setAccepted(false);
}
