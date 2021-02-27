#pragma once
#include <QSyntaxHighlighter>
#include <QPlainTextEdit>
#include "ScriptOperations.h"
#include "ScriptOperation.h"


class QOperationHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

private:
	QTextCharFormat highlightFormat, benchmarkFormat, errorFormat;
	ScriptOperations* operations;
	ScriptOperation* currentOperation;

public:
	QOperationHighlighter(QTextDocument* parent);
	~QOperationHighlighter();
	void setOperations(ScriptOperations* operations, ScriptOperation* currentOperation);

protected:
	void highlightBlock(const QString& text) override;
};
