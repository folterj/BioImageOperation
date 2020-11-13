/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <QDialog>
#include "ui_AboutWindow.h"


class AboutWindow : public QDialog
{
	Q_OBJECT

private:
	Ui::AboutWindow ui;

public:
	AboutWindow(QWidget *parent = Q_NULLPTR);
	~AboutWindow();

public slots:
	void linkActivated(const QString& link);
};
