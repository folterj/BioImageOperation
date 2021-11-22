/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <string>
#include <vector>

using namespace std;


/*
 * File path helper including filenames with numbers
 */

class NumericPath
{
public:
	vector<string> inputFilenames;
	string templatePath = "";
	string initialPath = "";
	string extension = "";
	string currentPath = "";
	int totaln = 0;
	int numlen = 0;
	int offset = 0;
	int filei = 0;
	bool set = false;
	bool input = true;

	NumericPath();
	void reset();
	void resetFilePath();
	bool setInputPath(string basepath, string templatePath);
	bool setOutputPath(string templatePath);
	bool setOutputPath(string basepath, string templatePath, string extra="", string defaultExtension = "", bool lookForNum = false);
	string createFilePath();
	string currentFilePath();
	string createFilePath(int i);
	bool isSet();
	int getFileCount();
	string getOriginalPath();
};
