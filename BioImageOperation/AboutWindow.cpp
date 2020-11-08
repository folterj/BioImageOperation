#include "AboutWindow.h"
#include <QDialogButtonBox>


AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent) {
	ui.setupUi(this);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QWidget::close);
}

AboutWindow::~AboutWindow() {
}
