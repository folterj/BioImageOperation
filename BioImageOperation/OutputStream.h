/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include <fstream>

using namespace std;


/*
 * Output stream helper
 */

class OutputStream : ofstream
{
public:
	string filename = "";
	bool isOpen = false;

	OutputStream();
	~OutputStream();
	void reset();
	void init(string filename, string header = "");
	void write(string output);
	void closeStream();
};
