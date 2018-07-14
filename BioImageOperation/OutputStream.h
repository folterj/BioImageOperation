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
#include <fstream>
#include <vcclr.h>


/*
 * Output stream helper
 */

class OutputStream : std::ofstream
{
public:
	gcroot<System::String^> filename = "";
	bool isOpen = false;

	OutputStream();
	~OutputStream();
	void reset();
	void init(System::String^ filename, System::String^ header);
	void write(System::String^ output);
	void closeStream();
};
