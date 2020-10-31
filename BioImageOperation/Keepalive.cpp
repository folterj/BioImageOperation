/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "KeepAlive.h"
#include "windows.h"


void Keepalive::startKeepAlive() {
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
}


void Keepalive::stopKeepAlive() {
	SetThreadExecutionState(ES_CONTINUOUS);
}
