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


class ScriptOperation;	// forward declaration


/*
 * Specific vector helper
 */

class ScriptOperations : vector<ScriptOperation*>
{
public:
	int currentOperationi = 0;

	ScriptOperations();
	~ScriptOperations();
	void reset();
	void extract(string script, int linei = 0);
	bool hasOperations();
	ScriptOperation* getCurrentOperation();
	bool moveNextOperation();
	void updateBenchmarking();
	void renderText(vector<string>* lines);
	void close();
};
