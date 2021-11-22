/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <filesystem>
#include "NumericPath.h"
#include "Util.h"


NumericPath::NumericPath() {
}

void NumericPath::reset() {
	inputFilenames.clear();
	templatePath = "";
	initialPath = "";
	extension = "";
	currentPath = "";
	totaln = 0;
	numlen = 0;
	offset = 0;
	filei = 0;
	set = false;
	input = true;
}

void NumericPath::resetFilePath() {
	filei = 0;
}

bool NumericPath::setInputPath(string basepath, string templatePath) {
	if (templatePath == "") {
		throw invalid_argument("No path specified");
	}

	templatePath = Util::combinePath(basepath, templatePath);

	reset();

	this->templatePath = templatePath;
	initialPath = templatePath;

	inputFilenames = Util::getImageFilenames(templatePath);
	totaln = (int)inputFilenames.size();

	extension = Util::extractFileExtension(templatePath);
	initialPath = Util::extractFilePath(templatePath);
	set = (totaln > 0);

	return set;
}

bool NumericPath::setOutputPath(string templatePath) {
	string extension = Util::extractFileExtension(templatePath);
	string title = Util::extractFileTitle(templatePath);
	string path = Util::extractFilePath(templatePath);
	if (!Util::endsWith(title, "_")) {
		templatePath = Util::combinePath(path, title + "_" + extension);
	}
	bool ok = setOutputPath("", templatePath);
	offset = 0;
	numlen = 1;
	return ok;
}

bool NumericPath::setOutputPath(string basepath, string templatePath, string extra, string defaultExtension, bool lookForNum) {
	int extPos;
	int numpos;
	int test;
	bool ok = true;

	if (templatePath == "") {
		throw invalid_argument("No path specified");
	}

	if (extra != "") {
		templatePath = Util::combinePath(Util::extractFilePath(templatePath), Util::replace(Util::extractFileTitle(extra), ".", "_") + "_" + Util::extractFileName(templatePath));
	}

	templatePath = Util::combinePath(basepath, templatePath);

	reset();
	input = false;

	this->templatePath = templatePath;

	extPos = (int)templatePath.find_last_of(".");
	if (extPos >= 0) {
		extension = templatePath.substr(extPos);
		numpos = extPos;
	} else if (defaultExtension != "") {
		// no extension found -> add
		if (!Util::startsWith(defaultExtension, ".")) {
			defaultExtension = "." + defaultExtension;
		}
		extension = defaultExtension;
		numpos = (int)templatePath.length();
	}

	if (lookForNum) {
		numpos--;
		while (ok) {
			try {
				test = stoi(templatePath.substr(numpos, numlen + 1));
				ok = true;
			} catch (...) {
				ok = false;
			}
			if (ok) {
				offset = test;
				numpos--;
				numlen++;
			} else {
				numpos++;
			}
		}
	}
	initialPath = templatePath.substr(0, numpos);

	set = filesystem::exists(Util::extractFilePath(initialPath));

	return set;
}

string NumericPath::createFilePath() {
	currentPath = createFilePath(filei++);
	return currentPath;
}

string NumericPath::currentFilePath() {
	return currentPath;
}

string NumericPath::createFilePath(int i) {
	string path = "";
	string format;

	if (input) {
		if (i < totaln) {
			path = inputFilenames[i];
		}
	} else {
		path = initialPath;
		int num = offset + i;
		if (numlen > 0) {
			format = "%0" + to_string(numlen) + "d";
			path += Util::format(format, num);
		}
		path += extension;
	}

	return path;
}

bool NumericPath::isSet() {
	return set;
}

int NumericPath::getFileCount() {
	return totaln;
}

string NumericPath::getOriginalPath() {
	return templatePath;
}
