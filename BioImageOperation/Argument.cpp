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

#include "Argument.h"
#include "Util.h"

using namespace System;


Argument::Argument(System::String^ arg)
{
	array<Char>^ quotes = { L'\"', L'\'' };
	int i;

	allArgument = Util::stdString(arg);

	i = arg->IndexOf("=");
	if (i > 0)
	{
		value = Util::stdString(arg->Substring(i + 1)->Trim()->Trim(quotes));
		arg = arg->Substring(0, i)->Trim();	// label
		if (!Enum::TryParse<ArgumentLabel>(arg, true, argumentLabel))
		{
			throw gcnew ArgumentException("Unkown argument: " + arg);
		}
	}
	else
	{
		if (arg->Contains("\"") || arg->Contains("\'"))
		{
			arg = arg->Trim(quotes);
			argumentLabel = ArgumentLabel::Path;
		}
		else if (Util::isNumeric(arg))
		{
			// prevent interpretation of number as enum
		}
		else
		{
			Enum::TryParse<ArgumentLabel>(arg, true, argumentLabel);
		}

		value = Util::stdString(arg);
	}
}
