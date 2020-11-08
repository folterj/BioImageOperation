#pragma once

#include <QDialog>
#include "ui_AboutWindow.h"

class AboutWindow : public QDialog
{
	Q_OBJECT

private:
	Ui::AboutWindow ui;

public:
	AboutWindow(QWidget *parent = Q_NULLPTR);
	~AboutWindow();
};
