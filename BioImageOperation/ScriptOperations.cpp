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
	ScriptOperation* operation = nullptr;
	vector<string> lines;
	string original, line;
	bool skipping = false;

	clear();

	this->script = script;
	lines = Util::split(script, "\n");

	for (int linei = linei0; linei < lines.size(); linei++) {
		original = lines[linei];
		line = Util::trim(original);
		if (Util::startsWith(line, "{")) {
			// adds inner instructions for last operation
			if (operation) {
				operation->innerOperations = new ScriptOperations();
				operation->innerOperations->extract(script, linei + 1);
				skipping = true;
			}
		} else if (Util::startsWith(line, "}")) {
			if (skipping) {
				if (operation) {
					operation->lineEnd = linei;
				}
				skipping = false;
			} else {
				return;
			}
		} else if (!skipping && line != "" && !Util::startsWith(line, "//") && !Util::startsWith(line, "#")) {
			try {
				operation = new ScriptOperation();
				operation->extract(original, line);
				operation->lineStart = linei;
				operation->lineEnd = linei;
				push_back(operation);
			} catch (exception& e) {
				throw invalid_argument(e.what() + string(" in line:\n") + line);
			}
		}
	}

	operationLineMap.clear();
	createOperationLineList(this);
}

void ScriptOperations::createOperationLineList(ScriptOperations* operations) {
	for (ScriptOperation* operation : *operations) {
		operationLineMap.emplace(operation->lineStart, operation);
		if (operation->hasInnerOperations()) {
			createOperationLineList(operation->innerOperations);		// * recursive
		}
	}
}

bool ScriptOperations::hasOperations() {
	return (size() > 0);
}

ScriptOperation* ScriptOperations::getCurrentOperation() {
	ScriptOperation* operation = nullptr;
	if (currentOperationi < size()) {
		operation = at(currentOperationi);
	}
	return operation;
}

ScriptOperation* ScriptOperations::getOperation(int linei) {
	auto item = operationLineMap.find(linei);
	if (item != operationLineMap.end()) {
		return item->second;
	}
	return nullptr;
}

bool ScriptOperations::moveNextOperation() {
	currentOperationi++;
	return (currentOperationi < size());
}

void  ScriptOperations::updateBenchmarking() {
	for (ScriptOperation* operation : *this) {
		operation->updateBenchmarking();
	}
}

string ScriptOperations::renderOperations() {
	string script1;
	vector<string> lines = Util::split(script, "\n");
	renderOperations(&lines);
	for (string line : lines) {
		script1 += line + "\n";
	}
	return script1;
}

void ScriptOperations::renderOperations(vector<string>* lines) {
	string text, line, extra;
	for (ScriptOperation* operation : *this) {
		line = (*lines)[operation->lineStart];
		extra = operation->extra;
		if (extra != "") {
			int i = (int)(80 - line.length() - extra.length());
			if (i < 1) {
				i = 1;
			}
			(*lines)[operation->lineStart] += string(i, ' ') + extra;
		}
		if (operation->hasInnerOperations()) {
			operation->innerOperations->renderOperations(lines);		// * recursive
		}
	}
}

void ScriptOperations::close() {
	for (int i = 0; i < size(); i++) {
		at(i)->close();
	}
}
