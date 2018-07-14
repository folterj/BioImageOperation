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

#include "NumericPath.h"
#include "Util.h"

using namespace System::IO;


NumericPath::NumericPath()
{
}

void NumericPath::reset()
{
	inputFilenames = nullptr;
	templatePath = "";
	initialPath = "";
	extension = "";
	currentPath = "";
	totaln = 0;
	numlen = 0;
	offset = 0;
	filei = 0;
	set = false;
	input = true;
}

void NumericPath::resetFilePath()
{
	filei = 0;
}

bool NumericPath::setInputPath(System::String^ basePath, System::String^ templatePath)
{
	if (templatePath == "")
	{
		throw gcnew System::Exception("No path specified");
	}

	templatePath = Util::combinePath(basePath, templatePath);

	reset();

	this->templatePath = templatePath;
	initialPath = templatePath;

	inputFilenames = Util::getImageFileNames(templatePath);
	Array::Sort(inputFilenames);
	totaln = inputFilenames->Length;

	extension = Util::extractFileExtension(templatePath);
	initialPath = Util::extractFilePath(templatePath);
	set = (totaln > 0);

	return set;
}

bool NumericPath::setOutputPath(System::String^ basePath, System::String^ templatePath, System::String^ defaultExtension)
{
	int extPos;
	int numpos;
	int test;
	bool ok = true;

	if (templatePath == "")
	{
		throw gcnew System::Exception("No path specified");
	}

	templatePath = Util::combinePath(basePath, templatePath);

	reset();
	input = false;

	this->templatePath = templatePath;

	extPos = templatePath->LastIndexOf(".");
	if (extPos >= 0)
	{
		extension = templatePath->Substring(extPos);
		numpos = extPos - 1;
	}
	else
	{
		if (!defaultExtension->StartsWith("."))
		{
			defaultExtension = "." + defaultExtension;
		}
		extension = defaultExtension;
		numpos = templatePath->Length - 1;
	}

	while (ok)
	{
		ok = int::TryParse(templatePath->Substring(numpos, numlen + 1), test);
		if (ok)
		{
			offset = test;
			numpos--;
			numlen++;
		}
		else
		{
			numpos++;
		}
	}
	initialPath = templatePath->Substring(0, numpos);

	set = Directory::Exists(Util::extractFilePath(initialPath));

	return set;
}

System::String^ NumericPath::createFilePath()
{
	currentPath = createFilePath(filei++);
	return currentFilePath();
}

System::String^ NumericPath::currentFilePath()
{
	return currentPath;
}

System::String^ NumericPath::createFilePath(int i)
{
	System::String^ path = "";
	System::String^ nums;

	if (input)
	{
		if (i < totaln)
		{
			path = inputFilenames[i];
		}
	}
	else
	{
		path = initialPath;
		int num = offset + i;
		if (numlen > 0)
		{
			nums = System::String::Format("{0}", num);
			path += nums->PadLeft(numlen, '0');
		}
		path += extension;
	}

	return path;
}

bool NumericPath::isSet()
{
	return set;
}

int NumericPath::getFileCount()
{
	return totaln;
}

System::String^ NumericPath::getOriginalPath()
{
	return templatePath;
}
