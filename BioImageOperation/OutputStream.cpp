/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include <filesystem>
#include "OutputStream.h"
#include "Util.h"


OutputStream::OutputStream() {
}

OutputStream::OutputStream(string filename) {
	this->filename = filename;
	exceptions(ofstream::failbit);
	open(filename, std::ios_base::out);
	isOpen = is_open();
}

OutputStream::~OutputStream() {
	closeStream();
}

void OutputStream::reset() {
	closeStream();
}

void OutputStream::init(string filename, string header) {
	bool fileExists;

	if (filename != this->filename) {
		closeStream();
		this->filename = filename;
	}

	if (!is_open()) {
		fileExists = filesystem::exists(filename);

		open(filename, std::ios_base::out | std::ios_base::app);

		if (!fileExists && header != "") {
			write(header);
		}
	}
	isOpen = is_open();
}

void OutputStream::write(string output) {
	if (is_open()) {
		(*this) << output;
	} else {
		isOpen = false;
		throw ios_base::failure("Unable to write to file " + filename);
	}
}

void OutputStream::closeStream() {
	if (isOpen) {
		if (is_open()) {
			flush();
			close();
		}
		isOpen = false;
	}
}
