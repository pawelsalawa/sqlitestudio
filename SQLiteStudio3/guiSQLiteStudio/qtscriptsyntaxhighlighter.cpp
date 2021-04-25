#include "qtscriptsyntaxhighlighter.h"
#include "style.h"
#include <QApplication>
#include <QStyle>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QtGui>

/**
 * @brief The JavaScript highlighter
 *
 * This class is mostly copied from Ofi Labs X2 project. It has been slightly modified for SQLiteStudio needs.
 * See the source code for the full license disclaimer.
 */
class GUI_API_EXPORT JavaScriptSyntaxHighlighter : public QSyntaxHighlighter
{
    public:
        explicit JavaScriptSyntaxHighlighter(QTextDocument *parent = 0);
        void mark(const QString &str, Qt::CaseSensitivity caseSensitivity);

    protected:
        void highlightBlock(const QString &text);

    private:
        void highlightTemplateExpressions(const QString &text, int strStart, int strEnd);

        QSet<QString> keywords;
        QSet<QString> knownIds;
        QString markString;
        Qt::CaseSensitivity markCaseSensitivity;
        QTextCharFormat normalFormat;
        QTextCharFormat keywordsFormat;
        QTextCharFormat commentFormat;
        QTextCharFormat stringFormat;
        QTextCharFormat expressionFormat;
};

JavaScriptSyntaxHighlighter::JavaScriptSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , markCaseSensitivity(Qt::CaseInsensitive)
{
    // https://developer.mozilla.org/en/JavaScript/Reference/Reserved_Words
    keywords << "break";
    keywords << "case";
    keywords << "catch";
    keywords << "continue";
    keywords << "default";
    keywords << "delete";
    keywords << "do";
    keywords << "else";
    keywords << "finally";
    keywords << "for";
    keywords << "function";
    keywords << "if";
    keywords << "in";
    keywords << "instanceof";
    keywords << "new";
    keywords << "return";
    keywords << "switch";
    keywords << "this";
    keywords << "throw";
    keywords << "try";
    keywords << "typeof";
    keywords << "var";
    keywords << "void";
    keywords << "while";
    keywords << "with";

    keywords << "true";
    keywords << "false";
    keywords << "null";

    // built-in and other popular objects + properties
    knownIds << "Object";
    knownIds << "prototype";
    knownIds << "create";
    knownIds << "defineProperty";
    knownIds << "defineProperties";
    knownIds << "getOwnPropertyDescriptor";
    knownIds << "keys";
    knownIds << "getOwnPropertyNames";
    knownIds << "constructor";
    knownIds << "__parent__";
    knownIds << "__proto__";
    knownIds << "__defineGetter__";
    knownIds << "__defineSetter__";
    knownIds << "eval";
    knownIds << "hasOwnProperty";
    knownIds << "isPrototypeOf";
    knownIds << "__lookupGetter__";
    knownIds << "__lookupSetter__";
    knownIds << "__noSuchMethod__";
    knownIds << "propertyIsEnumerable";
    knownIds << "toSource";
    knownIds << "toLocaleString";
    knownIds << "toString";
    knownIds << "unwatch";
    knownIds << "valueOf";
    knownIds << "watch";

    knownIds << "Function";
    knownIds << "arguments";
    knownIds << "arity";
    knownIds << "caller";
    knownIds << "constructor";
    knownIds << "length";
    knownIds << "name";
    knownIds << "apply";
    knownIds << "bind";
    knownIds << "call";

    knownIds << "String";
    knownIds << "fromCharCode";
    knownIds << "length";
    knownIds << "charAt";
    knownIds << "charCodeAt";
    knownIds << "concat";
    knownIds << "indexOf";
    knownIds << "lastIndexOf";
    knownIds << "localCompare";
    knownIds << "match";
    knownIds << "quote";
    knownIds << "replace";
    knownIds << "search";
    knownIds << "slice";
    knownIds << "split";
    knownIds << "substr";
    knownIds << "substring";
    knownIds << "toLocaleLowerCase";
    knownIds << "toLocaleUpperCase";
    knownIds << "toLowerCase";
    knownIds << "toUpperCase";
    knownIds << "trim";
    knownIds << "trimLeft";
    knownIds << "trimRight";

    knownIds << "Array";
    knownIds << "isArray";
    knownIds << "index";
    knownIds << "input";
    knownIds << "pop";
    knownIds << "push";
    knownIds << "reverse";
    knownIds << "shift";
    knownIds << "sort";
    knownIds << "splice";
    knownIds << "unshift";
    knownIds << "concat";
    knownIds << "join";
    knownIds << "filter";
    knownIds << "forEach";
    knownIds << "every";
    knownIds << "map";
    knownIds << "some";
    knownIds << "reduce";
    knownIds << "reduceRight";

    knownIds << "RegExp";
    knownIds << "global";
    knownIds << "ignoreCase";
    knownIds << "lastIndex";
    knownIds << "multiline";
    knownIds << "source";
    knownIds << "exec";
    knownIds << "test";

    knownIds << "JSON";
    knownIds << "parse";
    knownIds << "stringify";

    knownIds << "decodeURI";
    knownIds << "decodeURIComponent";
    knownIds << "encodeURI";
    knownIds << "encodeURIComponent";
    knownIds << "eval";
    knownIds << "isFinite";
    knownIds << "isNaN";
    knownIds << "parseFloat";
    knownIds << "parseInt";
    knownIds << "Infinity";
    knownIds << "NaN";
    knownIds << "undefined";

    knownIds << "Math";
    knownIds << "E";
    knownIds << "LN2";
    knownIds << "LN10";
    knownIds << "LOG2E";
    knownIds << "LOG10E";
    knownIds << "PI";
    knownIds << "SQRT1_2";
    knownIds << "SQRT2";
    knownIds << "abs";
    knownIds << "acos";
    knownIds << "asin";
    knownIds << "atan";
    knownIds << "atan2";
    knownIds << "ceil";
    knownIds << "cos";
    knownIds << "exp";
    knownIds << "floor";
    knownIds << "log";
    knownIds << "max";
    knownIds << "min";
    knownIds << "pow";
    knownIds << "random";
    knownIds << "round";
    knownIds << "sin";
    knownIds << "sqrt";
    knownIds << "tan";

    knownIds << "document";
    knownIds << "window";
    knownIds << "navigator";
    knownIds << "userAgent";

    keywordsFormat.setFontWeight(QFont::Bold);
    commentFormat.setFontItalic(true);
}

void JavaScriptSyntaxHighlighter::highlightBlock(const QString &text)
{
    // parsing state
    enum
    {
        Start = -1,
        Number = 1,
        Identifier = 2,
        String = 3,
        Comment = 4,
        Regex = 5
    };

    commentFormat.setForeground(QApplication::style()->standardPalette().dark());
    keywordsFormat.setForeground(QApplication::style()->standardPalette().windowText());
    keywordsFormat.setFontWeight(QFont::Bold);
    normalFormat.setForeground(QApplication::style()->standardPalette().text());
    stringFormat.setForeground(STYLE->extendedPalette().editorString());
    expressionFormat.setForeground(STYLE->extendedPalette().editorExpression());

    int state = previousBlockState();
    int start = 0;
    int i = 0;
    while (i <= text.length())
    {
        QChar ch = (i < text.length()) ? text.at(i) : QChar();
        QChar next = (i < text.length() - 1) ? text.at(i + 1) : QChar();

        switch (state)
        {
            case Start:
                start = i;
                if (ch.isSpace())
                {
                    ++i;
                }
                else if (ch.isDigit())
                {
                    ++i;
                    state = Number;
                }
                else if (ch.isLetter() || ch == '_')
                {
                    ++i;
                    state = Identifier;
                }
                else if (ch == '\'' || ch == '\"' || ch == '`')
                {
                    ++i;
                    state = String;
                }
                else if (ch == '/' && next == '*')
                {
                    ++i;
                    ++i;
                    state = Comment;
                }
                else if (ch == '/' && next == '/')
                {
                    i = text.length();
                    setFormat(start, text.length(), commentFormat);
                }
                else if (ch == '/' && next != '*')
                {
                    ++i;
                    state = Regex;
                }
                else
                {
                    if (!QString("(){}[]").contains(ch))
                        setFormat(start, 1, normalFormat);
                    ++i;
                    state = Start;
                }
                break;

            case Number:
                if (ch.isSpace() || !ch.isDigit())
                {
                    setFormat(start, i - start, normalFormat);
                    state = Start;
                }
                else
                    ++i;

                break;

            case Identifier:
                if (ch.isSpace() || !(ch.isDigit() || ch.isLetter() || ch == '_'))
                {
                    QString token = text.mid(start, i - start).trimmed();
                    if (keywords.contains(token))
                        setFormat(start, i - start, keywordsFormat);
                    else if (knownIds.contains(token))
                        setFormat(start, i - start, normalFormat);

                    state = Start;
                }
                else
                    ++i;

                break;

            case String:
                if (ch == text.at(start))
                {
                    QChar prev = (i > 0) ? text.at(i - 1) : QChar();
                    if (prev != '\\')
                    {
                        ++i;
                        setFormat(start, i - start, stringFormat);
                        if (ch == '`')
                            highlightTemplateExpressions(text, start, i);

                        state = Start;
                    }
                    else
                        ++i;
                }
                else
                    ++i;

                break;

            case Comment:
                if (ch == '*' && next == '/')
                {
                    ++i;
                    ++i;
                    setFormat(start, i - start, commentFormat);
                    state = Start;
                }
                else
                    ++i;

                break;

            case Regex:
                if (ch == '/')
                {
                    QChar prev = (i > 0) ? text.at(i - 1) : QChar();
                    if (prev != '\\')
                    {
                        ++i;
                        setFormat(start, i - start, expressionFormat);
                        state = Start;
                    }
                    else
                        ++i;
                }
                else
                    ++i;

                break;

            default:
                state = Start;
                break;
        }
    }

    if (state == Comment)
        setFormat(start, text.length(), commentFormat);
    else
        state = Start;

    if (!markString.isEmpty())
    {
        int pos = 0;
        int len = markString.length();
        QTextCharFormat markerFormat;
        markerFormat.setBackground(QApplication::style()->standardPalette().alternateBase());
        markerFormat.setForeground(QApplication::style()->standardPalette().text());
        for (;;)
        {
            pos = text.indexOf(markString, pos, markCaseSensitivity);
            if (pos < 0)
                break;
            setFormat(pos, len, markerFormat);
            ++pos;
        }
    }

    setCurrentBlockState(state);
}

void JavaScriptSyntaxHighlighter::highlightTemplateExpressions(const QString& text, int strStart, int strEnd)
{
    bool expr = false;
    int i = strStart;
    int start = i;
    while (i <= strEnd)
    {
        QChar ch = text.at(i);
        QChar next = (i < strEnd - 1) ? text.at(i + 1) : QChar();
        if (expr)
        {
            if (ch == '}')
            {
                QChar prev = (i > 0) ? text.at(i - 1) : QChar();
                if (prev != '\\')
                {
                    ++i;
                    setFormat(start, i - start, expressionFormat);
                    expr = false;
                }
                else
                    ++i;
            }
            else
                ++i;

        }
        else if (ch == '$' && next == '{')
        {
            start = i;
            expr = true;
            ++i;
            ++i;
        }
        else
        {
            ++i;
        }
    }
}

void JavaScriptSyntaxHighlighter::mark(const QString &str, Qt::CaseSensitivity caseSensitivity)
{
    markString = str;
    markCaseSensitivity = caseSensitivity;
    rehighlight();
}


QString JavaScriptHighlighterPlugin::getLanguageName() const
{
    return QStringLiteral("JavaScript");
}

QSyntaxHighlighter* JavaScriptHighlighterPlugin::createSyntaxHighlighter(QWidget* textEdit) const
{
    QPlainTextEdit* plainEdit = dynamic_cast<QPlainTextEdit*>(textEdit);
    if (plainEdit)
        return new JavaScriptSyntaxHighlighter(plainEdit->document());

    QTextEdit* edit = dynamic_cast<QTextEdit*>(textEdit);
    if (edit)
        return new JavaScriptSyntaxHighlighter(edit->document());

    return nullptr;
}
