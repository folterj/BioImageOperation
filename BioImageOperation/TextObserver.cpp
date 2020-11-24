/*****************************************************************************
 * Bio Image Operation (BIO)
 * Copyright (C) 2013-2020 Joost de Folter <folterj@gmail.com>
 * and the BIO developers.
 * This software is licensed under the terms of the GPL3 License.
 * See LICENSE.md in the project root folder for more information.
 * https://github.com/folterj/BioImageOperation
 *****************************************************************************/

#include "TextObserver.h"
#include "Constants.h"
#include "Util.h"


void TextObserver::resetProgressTimer() {
	time = Clock::now();
}

bool TextObserver::checkStatusProcess() {
	return true;
}

bool TextObserver::checkTextProcess(int displayi) {
	return true;
}

bool TextObserver::checkImageProcess(int displayi) {
	return true;
}

void TextObserver::resetUI() {
}

void TextObserver::clearStatus() {
	cout << endl;
}

void TextObserver::showStatus(int i, int tot, string label) {
	string s;
	double progress = 0;
	Clock::time_point now;
	chrono::duration<double> totalElapsed;
	double totalElapseds;
	double estimateLeft = 0;
	double avgFrametime;
	int processFps;

	try {
		now = Clock::now();
		totalElapsed = now - time;
		totalElapseds = totalElapsed.count();
		avgFrametime = totalElapseds / (i + 1);
		processFps = 1 / avgFrametime;

		if (tot > 0) {
			progress = (double)i / tot;
			s += Util::format("%.1f%%", 100 * progress);
		}
		if (progress > 0) {
			estimateLeft = totalElapseds * (1 / progress - 1);
		}
		s += Util::format(" %s (#%d) %.3fs @%dfps", label.c_str(), i, avgFrametime, processFps);
		s += " Elapsed: " + Util::formatTimespan((int)totalElapseds);
		if (estimateLeft > 0) {
			s += " Left: " + Util::formatTimespan((int)estimateLeft);
		}
		cout << s << "\r";
	} catch (exception e) {
		showDialog(Util::getExceptionDetail(e), (int)MessageLevel::Error);
	}
}

void TextObserver::showDialog(string message, int level) {
	cout << MessageLevels[level] + " " + message << endl;
}

void TextObserver::showText(string text, int displayi, string reference) {
	cout << Util::format("[%d] (%s) %s", displayi, reference.c_str(), text.c_str()) << endl;
}

void TextObserver::showImage(Mat* image, int displayi, string reference) {
}