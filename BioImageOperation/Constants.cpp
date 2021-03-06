/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "Constants.h"


const string Constants::defaultScriptExtension = "bioscript";
const string Constants::defaultHelpExtension = "md";
const string Constants::defaultDataExtension = "csv";
const string Constants::defaultImageExtension = "png";
const string Constants::defaultVideoExtension = "mp4";
const string Constants::defaultVideoCodec = "H264";
const string Constants::scriptFileDialogFilter = "BIO Script files (*." + defaultScriptExtension + ")";
const string Constants::scriptHelpDialogFilter = "BIO script help (*." + defaultHelpExtension + ")";
const int Constants::defaultScriptFileDialogFilter = 1;
const string Constants::scriptHelpDefaultFilename = "BioImageOperation script." + defaultHelpExtension;
const string Constants::webPage = "https://joostdefolter.info";
const string Constants::webFilesUrl = webPage + "/files/";

const string Constants::scriptTemplatePath = ":/BioImageOperation/templates/";
const string Constants::trackingScriptTemplate = scriptTemplatePath + "template_tracking." + defaultScriptExtension;
const string Constants::filenameTemplate = "<filename>";

const int Constants::seekModeInterval = 100;
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
