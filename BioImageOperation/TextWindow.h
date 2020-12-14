/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <QMainWindow>
#include "ui_TextWindow.h"
#include "Observer.h"


class TextWindow : public QMainWindow {
	Q_OBJECT

private:
	Ui::TextWindow ui;
	Observer* observer;
	string reference;
	int title;

public:
	TextWindow(QWidget *parent = Q_NULLPTR);
	~TextWindow();
	void init(Observer* observer, int title = 0);
	void updateTitle();
	void showText(string text, string reference = "");
};
