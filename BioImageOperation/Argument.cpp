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
		value = Util::removeQuotes(Util::trim_copy(arg.substr(i + 1)));
		arg = Util::trim_copy(arg.substr(0, i));	// label
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

bool Argument::checkType(ArgumentType argumentType) {
	bool ok = true;
	double x;

	switch (argumentType) {
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

	case ArgumentType::Display:
		ok = Util::isNumeric(value);
		if (ok) {
			x = stoi(value);
			ok = (x >= 0 && x < Constants::nDisplays);
		}
		break;

	case ArgumentType::Bool:
		if (value != "") {
			ok = Util::isBoolean(value);
		}
		break;

	case ArgumentType::Codec:
		ok = (value.size() == 4);
		break;

	}

	return ok;
}
