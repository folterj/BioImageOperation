#include "QOperationHighlighter.h"


QOperationHighlighter::QOperationHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent) {
	highlightFormat.setBackground(Qt::yellow);
	benchmarkFormat.setForeground(Qt::gray);
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
	ScriptOperation* operation = operations->getOperation(linei);
	int toti = text.length();
	int ii = toti;

	if (operation) {
		ii = operation->original.length();
	}
	if (linei >= currentOperation->lineStart && linei <= currentOperation->lineEnd) {
		setFormat(0, ii, highlightFormat);
	}
	if (operation && toti > ii) {
		setFormat(ii, toti, benchmarkFormat);
	}

	setCurrentBlockState(linei);
}
