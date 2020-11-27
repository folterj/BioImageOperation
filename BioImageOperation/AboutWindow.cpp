/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "AboutWindow.h"
#include <QDialogButtonBox>
#include <string>
#include <opencv2/opencv.hpp>
#include "Util.h"
#include "config.h"

using namespace std;


AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent) {	
	string html = string(PROJECT_NAME) + "<br><br>"
				+ "Version " + string(PROJECT_VER) + "<br><br>"
				+ "(OpenCV Version " + cv::getVersionString() + ")<br><br>"
				+ string(PROJECT_DESC) + "<br><br>"
				+ "<a href=" + string(PROJECT_URL) + ">" + string(PROJECT_URL) + "</a>";
	ui.setupUi(this);
	ui.infoLabel->setText(Util::convertToQString(html));
	connect(ui.infoLabel, &QLabel::linkActivated, this, &AboutWindow::linkActivated);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QWidget::close);
}

AboutWindow::~AboutWindow() {
}

void AboutWindow::linkActivated(const QString& link) {
	Util::openWebLink(link.toStdString());
}
