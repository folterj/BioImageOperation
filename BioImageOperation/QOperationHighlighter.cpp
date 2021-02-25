#include "QOperationHighlighter.h"


QOperationHighlighter::QOperationHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent) {
	highlightFormat.setBackground(Qt::yellow);
	errorFormat.setUnderlineStyle(QTextCharFormat::UnderlineStyle::WaveUnderline);
}

QOperationHighlighter::~QOperationHighlighter() {
}

void QOperationHighlighter::setOperations(ScriptOperations* operations, ScriptOperation* currentOperation) {
	this->operations = operations;
	this->currentOperation = currentOperation;
}

void QOperationHighlighter::highlightBlock(const QString& text) {
	int linei = previousBlockState() + 1;
	if (linei >= currentOperation->lineStart && linei <= currentOperation->lineEnd) {
		setFormat(0, text.length(), highlightFormat);
	}

	setCurrentBlockState(linei);
}
