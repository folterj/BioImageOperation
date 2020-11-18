/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

// Cross-platform deployment
// https://doc.qt.io/qt-5/deployment.html
// https://stackoverflow.com/questions/43862542/distributing-qt-c-application-cross-platform
// vcpkg: a C++ package manager for Windows, Linux, and macOS: https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=msvc-160

#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "config.h"

Q_DECLARE_METATYPE(string)


int main(int argc, char *argv[]) {

	qRegisterMetaType<string>("string");

	QApplication application(argc, argv);
	application.setApplicationName(PROJECT_NAME);
	application.setApplicationVersion(PROJECT_VER);
	application.setOrganizationDomain(PROJECT_URL);
	MainWindow window;
	window.show();
	return application.exec();
}
