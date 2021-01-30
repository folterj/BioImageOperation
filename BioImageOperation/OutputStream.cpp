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
#include "Constants.h"


OutputStream::OutputStream(string filename, string header) {
	if (filename != "") {
		init(filename, header);
	}
}

OutputStream::~OutputStream() {
	reset();
}

void OutputStream::reset() {
	closeStream();
	filename = "";
	buffer = "";
	created = false;
}

void OutputStream::init(string filename, string header) {
	this->filename = filename;
	exceptions(ofstream::failbit | ofstream::badbit);
	if (header != "") {
		write(header);
	}
}

void OutputStream::write(string output) {
	if (output != "") {
		buffer += output;
		if (created && buffer.size() < Constants::maxLogBuffer) {
			return;
		}
	}
	if (buffer != "") {
		ios_base::openmode openMode = std::ios_base::out;
		if (created) {
			openMode |= std::ios_base::app;
		}
		open(filename, openMode);
		if (is_open()) {
			(*this) << buffer;
			flush();
			close();
			buffer = "";
			created = true;
		} else {
			throw ios_base::failure("Unable to write to file " + filename);
		}
	}
}

void OutputStream::closeStream() {
	write("");
}
