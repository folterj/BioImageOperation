/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once
#include <vector>

using namespace System;


class ScriptOperation;	// forward declaration


/*
 * Specific vector helper
 */

class ScriptOperations : std::vector<ScriptOperation*>
{
public:
	int currentOperationi = 0;

	ScriptOperations();
	~ScriptOperations();
	void reset();
	void extract(System::String^ script, int linei = 0);
	bool hasOperations();
	ScriptOperation* getCurrentOperation();
	bool moveNextOperation();
	void close();
};
