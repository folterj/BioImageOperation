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

#include "Constants.h"


const int Constants::nDisplays = 4;
const int Constants::nInfoWindows = 4;
const int Constants::nTrackers = 4;
const int Constants::seekModeInterval = 100;

const std::string Constants::webPage = "http://joostdefolter.info";
const std::string Constants::webFilesUrl = webPage + "/files/";
const std::string Constants::scriptFileDialogFilter = "BIO Script files (*.bioscript)|*.bioscript|All files (*.*)|*.*";
const int Constants::defaultScriptFileDialogFilter = 1;
const std::string Constants::defaultDataExtension = "csv";
const std::string Constants::defaultImageExtension = "png";
const std::string Constants::defaultVideoExtension = "mp4";
const std::string Constants::defaultVideoCodec = "H264";

const int Constants::minPixels = 2;
const int Constants::maxMergedBlobs = 5;
const int Constants::minPathDistance = 2;

const int Constants::clusterTrainingCycles = 10;
const int Constants::trackTrainingCycles = 10;
const int Constants::trainingDataPoints = 1000;
const int Constants::defMinActive = 3;
const int Constants::defMaxInactive = 3;
const int Constants::defMaxMove = 0;
const double Constants::maxBinaryPixelsFactor = 0.1;	// realistic value 0.01;
