/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "OutputStreams.h"


OutputStreams::~OutputStreams() {
	close();
}

void OutputStreams::close() {
	for (OutputStream* stream : *this) {
		stream->closeStream();
	}
	clear();
}

OutputStream* OutputStreams::get(string filename, string header) {
	for (OutputStream* stream : *this) {
		if (stream->filename == filename) {
			return stream;
		}
	}
	OutputStream* newOutputStream = new OutputStream(filename, header);
	push_back(newOutputStream);
	return newOutputStream;
}
