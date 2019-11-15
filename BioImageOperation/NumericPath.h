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


/*
 * File path helper including filenames with numbers
 */

public ref class NumericPath
{
public:
	array<System::String^>^ inputFilenames;
	System::String^ templatePath = "";
	System::String^ initialPath = "";
	System::String^ extension = "";
	System::String^ currentPath = "";
	int totaln = 0;
	int numlen = 0;
	int offset = 0;
	int filei = 0;
	bool set = false;
	bool input = true;

	NumericPath();
	void reset();
	void resetFilePath();
	bool setInputPath(System::String^ basePath, System::String^ templatePath);
	bool setOutputPath(System::String^ templatePath);
	bool setOutputPath(System::String^ basePath, System::String^ templatePath, System::String^ defaultExtension);
	System::String^ createFilePath();
	System::String^ currentFilePath();
	System::String^ createFilePath(int i);
	bool isSet();
	int getFileCount();
	System::String^ getOriginalPath();
};
