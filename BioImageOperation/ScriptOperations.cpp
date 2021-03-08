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

void ScriptOperations::extract(string script) {
	vector<string> lines;
	string line;
	bool useIndent = false;
	int nopenBrackets = 0;
	int ncloseBrackets = 0;
	int indentLevel = 0;

	this->script = script;
	lines = Util::split(script, "\n");
	for (string line0 : lines) {
		line = Util::trim(line0);
		if (Util::endsWith(line, ":")) {
			useIndent = true;
		}
		if (Util::endsWith(line, "{")) {
			nopenBrackets++;
		}
		if (Util::startsWith(line, "}")) {
			ncloseBrackets++;
		}
	}
	if (!lines.empty()) {
		line = lines[0];
		indentLevel = Util::getIndentLevel(line);
	}
	if (ncloseBrackets != nopenBrackets) {
		throw invalid_argument("Mismatched number of open/close brackets { }");
	}
	extract(lines, 0, indentLevel, useIndent);
	operationLineMap.clear();
	createOperationLineList(this);
}

int ScriptOperations::extract(vector<string> lines, int startlinei, int startIndentLevel, bool useIndent) {
	ScriptOperation* operation = nullptr;
	string original, line;
	bool indentUp, indentDown;
	int prevIndentLevel = startIndentLevel;
	int nextLineOffset = 0;
	int indentLevel;

	clear();

	for (int linei = startlinei; linei < lines.size(); linei++) {
		original = lines[linei];
		indentLevel = Util::getIndentLevel(original);
		line = Util::trim(original);

		if (line != "" && !Util::startsWith(line, "//") && !Util::startsWith(line, "#")) {
			indentUp = Util::endsWith(line, "{") || Util::endsWith(line, ":");
			if (indentUp) {
				line = line.substr(0, line.length() - 1);
			}
			indentDown = Util::startsWith(line, "}");
			if (indentDown) {
				line = line.substr(1);
			}
			if (useIndent && indentLevel < prevIndentLevel) {
				indentDown = true;
				nextLineOffset = -1;
			}

			if (indentDown) {
				return linei + nextLineOffset;
			} else if (line != "") {
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
			if (indentUp) {
				// adds inner instructions for last operation
				if (operation) {
					operation->innerOperations = new ScriptOperations();
					linei = operation->innerOperations->extract(lines, linei + 1, indentLevel, useIndent);
					operation->lineEnd = linei;
				}
			}
			prevIndentLevel = indentLevel;
		}
	}
	return lines.size();
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
