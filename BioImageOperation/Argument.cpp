/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "Argument.h"
#include "Util.h"
#include "Constants.h"


Argument::Argument(string arg) {
	int i;
	ArgumentLabel label;

	allArgument = arg;

	i = arg.find("=");
	if (i > 0) {
		value = Util::removeQuotes(Util::trim(arg.substr(i + 1)));
		arg = Util::trim(arg.substr(0, i));	// label
		label = getArgumentLabel(arg);
		if (label != ArgumentLabel::None) {
			argumentLabel = getArgumentLabel(arg);
		} else {
			throw invalid_argument("Unkown argument: " + arg);
		}
	} else {
		if (Util::contains(arg, "\"") || Util::contains(arg, "'")) {
			arg = Util::removeQuotes(arg);
			argumentLabel = ArgumentLabel::Path;
		} else if (Util::isNumeric(arg)) {
			// prevent interpretation of number as enum
		} else {
			argumentLabel = getArgumentLabel(arg);
		}

		value = arg;
	}
}

ArgumentLabel Argument::getArgumentLabel(string arg) {
	ArgumentLabel label = ArgumentLabel::None;
	int enumIndex = Util::getListIndex(ArgumentLabels, arg);
	if (enumIndex >= 0) {
		label = (ArgumentLabel)enumIndex;
	}
	return label;
}

bool Argument::parseType(ArgumentType argumentType) {
	bool ok = false;
	double x;
	vector<string> parts;

	valueEnum = -1;

	switch (argumentType) {
	case ArgumentType::Path:
	case ArgumentType::Label:
		ok = true;
		break;

	case ArgumentType::Num:
		ok = Util::isNumeric(value);
		break;

	case ArgumentType::Fraction:
		ok = Util::isNumeric(value);
		if (ok) {
			x = stof(value);
			ok = (x >= 0 && x <= 1);
		}
		break;

	case ArgumentType::TimeFrame:
		if (value.find(":") >= 0) {
			parts = Util::split(value, ":");
			for (string part : parts) {
				if (part != "") {
					ok = Util::isNumeric(value);
					if (!ok) {
						break;
					}
				}
			}
		} else {
			ok = Util::isNumeric(value);
		}
		break;

	case ArgumentType::Display:
		ok = Util::isNumeric(value);
		if (ok) {
			x = stoi(value);
			ok = (x >= 0 && x < Constants::nDisplays);
		}
		break;

	case ArgumentType::Bool:
		// no value is valid (interpreted as true)
		if (value != "") {
			ok = Util::isBoolean(value);
		}
		break;

	case ArgumentType::Codec:
		ok = (value.size() == 4);
		break;

	case ArgumentType::AccumMode:
		valueEnum = Util::getListIndex(AccumModes, value);
		break;

	case ArgumentType::ColorMode:
		valueEnum = Util::getListIndex(ImageColorModes, value);
		break;

	case ArgumentType::DrawMode:
		valueEnum = parseClusterDrawMode(value);
		break;

	case ArgumentType::Format:
		valueEnum = Util::getListIndex(SaveFormats, value);
		break;

	case ArgumentType::Palette:
		valueEnum = Util::getListIndex(Palettes, value);
		break;

	case ArgumentType::PathDrawMode:
		valueEnum = Util::getListIndex(PathDrawModes, value);
		break;

	case ArgumentType::Position:
		valueEnum = Util::getListIndex(DrawPositions, value);
		break;
	}
	if (valueEnum >= 0) {
		ok = true;
	}
	return ok;
}

int Argument::parseClusterDrawMode(string value) {
	int clusterDrawMode = 0;
	int clusterDrawMode0;
	vector<string> args = Util::split(value, vector<string>{"|", "&", "+"}, true);
	string arg;

	for (string arg0 : args) {
		arg = Util::trim(arg0);
		clusterDrawMode0 = Util::getListIndex(ClusterDrawModes, arg);
		if (clusterDrawMode0 >= 0) {
			clusterDrawMode |= (1 << (clusterDrawMode0 - 1));
		} else {
			return -1;
		}
	}

	return clusterDrawMode;
}
