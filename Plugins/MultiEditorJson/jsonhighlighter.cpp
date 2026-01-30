#include "jsonhighlighter.h"
#include <QColor>

JsonHighlighter::JsonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Key format (property names in quotes followed by colon)
    keyFormat.setForeground(QColor(86, 156, 214)); // Light blue
    keyFormat.setFontWeight(QFont::Bold);

    // String format
    stringFormat.setForeground(QColor(206, 145, 120)); // Light orange/salmon

    // Number format
    numberFormat.setForeground(QColor(181, 206, 168)); // Light green

    // Boolean format
    booleanFormat.setForeground(QColor(86, 156, 214)); // Light blue
    booleanFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b(true|false)\\b");
    rule.format = booleanFormat;
    highlightingRules.append(rule);

    // Null format
    nullFormat.setForeground(QColor(86, 156, 214)); // Light blue
    nullFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\bnull\\b");
    rule.format = nullFormat;
    highlightingRules.append(rule);

    // Number format (integers and floats)
    rule.pattern = QRegularExpression("-?\\b\\d+\\.?\\d*([eE][+-]?\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Braces format
    braceFormat.setForeground(QColor(220, 220, 170)); // Yellow
    rule.pattern = QRegularExpression("[{}]");
    rule.format = braceFormat;
    highlightingRules.append(rule);

    // Bracket format
    bracketFormat.setForeground(QColor(220, 220, 170)); // Yellow
    rule.pattern = QRegularExpression("[\\[\\]]");
    rule.format = bracketFormat;
    highlightingRules.append(rule);

    // Colon format
    colonFormat.setForeground(QColor(212, 212, 212)); // Light gray
    rule.pattern = QRegularExpression(":");
    rule.format = colonFormat;
    highlightingRules.append(rule);

    // Comma format
    commaFormat.setForeground(QColor(212, 212, 212)); // Light gray
    rule.pattern = QRegularExpression(",");
    rule.format = commaFormat;
    highlightingRules.append(rule);
}

void JsonHighlighter::highlightBlock(const QString &text)
{
    // First apply all simple rules
    for (const HighlightingRule &rule : highlightingRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle strings and keys separately (more complex logic)
    // Pattern for strings: "any text"
    QRegularExpression stringPattern("\"([^\"\\\\]|\\\\.)*\"");
    QRegularExpressionMatchIterator stringIterator = stringPattern.globalMatch(text);
    
    while (stringIterator.hasNext())
    {
        QRegularExpressionMatch match = stringIterator.next();
        int start = match.capturedStart();
        int length = match.capturedLength();
        
        // Check if this string is followed by a colon (making it a key)
        bool isKey = false;
        int checkPos = start + length;
        while (checkPos < text.length() && text[checkPos].isSpace())
            checkPos++;
        
        if (checkPos < text.length() && text[checkPos] == ':')
            isKey = true;
        
        // Apply appropriate format
        if (isKey)
            setFormat(start, length, keyFormat);
        else
            setFormat(start, length, stringFormat);
    }
}
