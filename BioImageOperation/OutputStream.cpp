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

#include "OutputStream.h"
#include "Util.h"

using namespace System::IO;


OutputStream::OutputStream()
{
}

OutputStream::~OutputStream()
{
	closeStream();
}

void OutputStream::reset()
{
	closeStream();
}

void OutputStream::init(System::String^ filename, System::String^ header)
{
	bool fileExists;

	this->filename = filename;

	if (!is_open())
	{
		fileExists = File::Exists(filename);

		open(Util::stdString(filename), std::ios_base::out | std::ios_base::app);

		if (!fileExists)
		{
			write(header);
		}
	}

	isOpen = is_open();
}

void OutputStream::write(System::String^ output)
{
	if (is_open())
	{
		(*this) << Util::stdString(output);
	}
	else
	{
		isOpen = false;
		throw gcnew System::Exception("Unable to write to file " + filename);
	}
}

void OutputStream::closeStream()
{
	if (isOpen)
	{
		if (is_open())
		{
			flush();
			close();
		}
		isOpen = false;
	}
}
