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

#include "ScriptOperations.h"
#include "ScriptOperation.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


ScriptOperations::ScriptOperations()
{
}

ScriptOperations::~ScriptOperations()
{
	for (int i = 0; i < size(); i++)
	{
		delete at(i);
	}
}

void ScriptOperations::reset()
{
	currentOperationi = 0;

	for (int i = 0; i < size(); i++)
	{
		at(i)->reset();
	}
}

void ScriptOperations::extract(System::String^ script, int linei0)
{
	ScriptOperation* operation = NULL;
	array<System::String^>^ lines = script->Split(gcnew array<System::String^>{ "\r\n", "\r", "\n" }, StringSplitOptions::None);
	System::String^ line;
	bool skipping = false;

	clear();

	for (int linei = linei0; linei < lines->Length; linei++)
	{
		line = lines[linei]->Trim();
		if (line->StartsWith("{"))
		{
			// adds inner instructions for last operation
			if (operation)
			{
				operation->innerOperations = new ScriptOperations();
				operation->innerOperations->extract(script, linei + 1);
				skipping = true;
			}
		}
		else if (line->StartsWith("}"))
		{
			if (skipping)
			{
				if (operation)
				{
					operation->lineEnd = linei;
				}
				skipping = false;
			}
			else
			{
				return;
			}
		}
		else if (!skipping && line != "" && !line->StartsWith("//") && !line->StartsWith("#"))
		{
			try
			{
				operation = new ScriptOperation();
				operation->extract(line);
				operation->lineStart = linei;
				operation->lineEnd = linei;
				push_back(operation);
			}
			catch (ArgumentException^ e)
			{
				throw gcnew ArgumentException(e->Message + " in line:\n" + line);
			}
		}
	}
}

bool ScriptOperations::hasOperations()
{
	return (size() > 0);
}

ScriptOperation* ScriptOperations::getCurrentOperation()
{
	ScriptOperation* operation = NULL;
	if (currentOperationi < size())
	{
		operation = at(currentOperationi);
	}
	return operation;
}

bool ScriptOperations::moveNextOperation()
{
	currentOperationi++;
	return (currentOperationi < size());
}

void ScriptOperations::close()
{
	for (int i = 0; i < size(); i++)
	{
		at(i)->close();
	}
}
