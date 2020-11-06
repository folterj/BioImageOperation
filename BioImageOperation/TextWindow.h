#pragma once

#include <QMainWindow>
#include "ui_TextWindow.h"
#include "Observer.h"


class TextWindow : public QMainWindow {
	Q_OBJECT

private:
	Ui::TextWindow ui;
	Observer* observer;
	int title;

public:
	TextWindow(QWidget *parent = Q_NULLPTR);
	~TextWindow();
	void init(Observer* observer, int title);
	void updateTitle();
	void showText(const char* text);
};
