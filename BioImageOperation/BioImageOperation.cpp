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

#include <QtWidgets/QApplication>
#include "config.h"
#include "ScriptProcessing.h"
#include "MainWindow.h"
#include "Util.h"

Q_DECLARE_METATYPE(string)


int main(int argc, char *argv[]) {
	bool hasGui = (argc <= 1);
	bool showUsage = false;
	string arg, arg2, min_arg;

	try {
        qRegisterMetaType<string>("string");

		if (hasGui) {
			QApplication application(argc, argv);
            application.setWindowIcon(QIcon(":/BioImageOperation/BioImageOperation.png"));
			application.setApplicationName(PROJECT_NAME);
			application.setApplicationVersion(PROJECT_VER);
			application.setOrganizationDomain(PROJECT_URL);
			MainWindow window;
			window.show();
			return application.exec();
		} else {
			arg = argv[1];
			cout << PROJECT_NAME << " version " << PROJECT_VER << " / " << PROJECT_DESC << "\n" << PROJECT_URL << "\n" << endl;
			if (Util::startsWith(arg, "-") || arg == "/help") {
				min_arg = Util::replace(arg, "--", "-");
				min_arg = Util::replace(min_arg, "-", "");
				min_arg = Util::replace(min_arg, "/", "");
				if (min_arg == "help") {
					if (argc > 2) {
						arg2 = argv[2];
						if (arg2 == "list") {
							cout << ScriptOperation::getOperationListSimple() << endl;
						} else if (arg2 == "all") {
							cout << ScriptOperation::getOperationList(false) << endl;
						} else {
							cout << ScriptOperation::getOperationList(false, arg2) << endl;
						}
					} else {
						showUsage = true;
					}
				} else if (min_arg != "version") {
					cout << "Invalid switch: " << arg << endl;
					showUsage = true;
				}
			} else {
				ScriptProcessing scriptProcessing;
				scriptProcessing.startProcessNoGui(arg);
			}
			if (showUsage) {
				cout << "Usage: " << PROJECT_NAME << " /path/to/script.bioscript\nScript help usage: -help list / -help [operation] / -help all" << endl;
			}
		}
	} catch (exception e) {
		cerr << e.what() << endl;
	}
}
