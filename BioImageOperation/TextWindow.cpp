/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TextWindow.h"
#include "Util.h"


TextWindow::TextWindow(QWidget *parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
}

TextWindow::~TextWindow() {
}

void TextWindow::init(Observer* observer, int title) {
	this->observer = observer;
	this->title = title;
	updateTitle();
}

void TextWindow::updateTitle() {
	string s = Util::format("BIO info %d", title);
	if (reference != "") {
		s += " (" + reference + ")";
	}
	setWindowTitle(Util::convertToQString(s));
}

void TextWindow::showText(string text, string reference) {
	if (isHidden()) {
		show();
	}
	this->reference = reference;
	ui.textEdit->setPlainText(Util::convertToQString(text));
	updateTitle();
}
