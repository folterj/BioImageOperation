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
#include <map>

using namespace std;


class ScriptOperation;	// forward declaration


/*
 * Specific vector helper
 */

class ScriptOperations : vector<ScriptOperation*>
{
private:
	string script;
	map<int, ScriptOperation*> operationLineMap;
	int currentOperationi = 0;

public:
	ScriptOperations();
	~ScriptOperations();
	void reset();
	void extract(string script);
	int extract(vector<string> lines, int startlinei, int startIndentLevel, bool useIndent);
	void createOperationLineList(ScriptOperations* operations);
	bool hasOperations();
	ScriptOperation* getCurrentOperation();
	ScriptOperation* getOperation(int linei);
	bool moveNextOperation();
	void updateBenchmarking();
	string renderOperations();
	void renderOperations(vector<string>* lines);
	void close();
};
