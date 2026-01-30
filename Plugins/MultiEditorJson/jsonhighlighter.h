#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class JsonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JsonHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keyFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat booleanFormat;
    QTextCharFormat nullFormat;
    QTextCharFormat braceFormat;
    QTextCharFormat bracketFormat;
    QTextCharFormat colonFormat;
    QTextCharFormat commaFormat;
};

#endif // JSONHIGHLIGHTER_H
