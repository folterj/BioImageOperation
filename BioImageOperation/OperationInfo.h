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
#include "Argument.h"

using namespace System;
using namespace System::Collections::Generic;


/*
 * Basic template for operation parameter definition and checking (also used for generation of script manual)
 */

ref class OperationInfo
{
public:
	array<ArgumentLabel>^ requiredArguments = nullptr;
	array<ArgumentLabel>^ optionalArguments = nullptr;
	System::String^ description = "";

	OperationInfo(array<ArgumentLabel>^ requiredArguments, array<ArgumentLabel>^ optionalArguments, System::String^ description);
};
