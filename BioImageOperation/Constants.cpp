/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
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
const int Constants::defMaxMove = 1000;
const double Constants::maxBinaryPixelsFactor = 0.1;	// realistic value 0.01;
