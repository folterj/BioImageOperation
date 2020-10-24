/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#pragma once
#include "Argument.h"


/*
 * Basic template for operation parameter definition and checking (also used for generation of script manual)
 */

class OperationInfo
{
public:
	vector<ArgumentLabel> requiredArguments;
	vector<ArgumentLabel> optionalArguments;
	string description = "";

	OperationInfo(vector<ArgumentLabel> requiredArguments, vector<ArgumentLabel> optionalArguments, string description);
};
