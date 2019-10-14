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

#pragma unmanaged
#include "opencv2/opencv.hpp"
#pragma managed

using namespace cv;
using namespace System;


/*
 * Observer interface to update back to the UI
 */

public interface class Observer
{
	void resetUI();
	void resetImages();
	void resetProgressTimer();
	void clearStatus();
	void showStatus(int i);
	void showStatus(int i, int tot);
	void showStatus(System::String^ label, int i, int tot, bool showFrameProgress);
	void showStatus(System::String^ status, double progress);
	void showInfo(System::String^ info, int displayi);
	void displayImage(Mat* image, int displayi);
	void showErrorMessage(System::String^ message);
};
