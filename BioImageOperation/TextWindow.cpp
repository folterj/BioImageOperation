#include "TextWindow.h"
#include "Util.h"

TextWindow::TextWindow(QWidget *parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
}

TextWindow::~TextWindow() {
}

void TextWindow::init(Observer* observer, int title) {
	this->observer = observer;
	this->title = title;
	updateTitle();
}

void TextWindow::updateTitle() {
	string s = Util::format("BIO info %d", title);
	setWindowTitle(Util::convertToQString(s));
}

void TextWindow::showText(const char* text) {
	if (isHidden()) {
		show();
	}

	ui.textEdit->setPlainText(text);
}
