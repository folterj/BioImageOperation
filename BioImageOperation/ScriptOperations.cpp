/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "ScriptOperations.h"
#include "ScriptOperation.h"
#include "Util.h"


ScriptOperations::ScriptOperations() {
}

ScriptOperations::~ScriptOperations() {
	for (int i = 0; i < size(); i++) {
		delete at(i);
	}
}

void ScriptOperations::reset() {
	currentOperationi = 0;

	for (int i = 0; i < size(); i++) {
		at(i)->reset();
	}
}

void ScriptOperations::extract(string script, int linei0) {
	ScriptOperation* operation = NULL;
	vector<string> lines = Util::split(script, string("\r\n"));
	string line;
	bool skipping = false;

	clear();

	for (int linei = linei0; linei < lines.size(); linei++) {
		line = Util::trim_copy(lines[linei]);
		if (line._Starts_with("{")) {
			// adds inner instructions for last operation
			if (operation) {
				operation->innerOperations = new ScriptOperations();
				operation->innerOperations->extract(script, linei + 1);
				skipping = true;
			}
		} else if (line._Starts_with("}")) {
			if (skipping) {
				if (operation) {
					operation->lineEnd = linei;
				}
				skipping = false;
			} else {
				return;
			}
		} else if (!skipping && line != "" && !line._Starts_with("//") && !line._Starts_with("#")) {
			try {
				operation = new ScriptOperation();
				operation->extract(line);
				operation->lineStart = linei;
				operation->lineEnd = linei;
				push_back(operation);
			} catch (exception e) {
				throw invalid_argument(e.what() + string(" in line:\n") + line);
			}
		}
	}
}

bool ScriptOperations::hasOperations() {
	return (size() > 0);
}

ScriptOperation* ScriptOperations::getCurrentOperation() {
	ScriptOperation* operation = NULL;
	if (currentOperationi < size()) {
		operation = at(currentOperationi);
	}
	return operation;
}

bool ScriptOperations::moveNextOperation() {
	currentOperationi++;
	return (currentOperationi < size());
}

void ScriptOperations::close() {
	for (int i = 0; i < size(); i++) {
		at(i)->close();
	}
}
