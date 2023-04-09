#include "qtscriptsyntaxhighlighter.h"
#include "style.h"
#include "uiconfig.h"
#include "common/global.h"
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
        explicit JavaScriptSyntaxHighlighter(QTextDocument *parent, const QHash<JavaScriptHighlighterPlugin::State, QTextCharFormat>* formats);

    protected:
        void highlightBlock(const QString &text);

    private:
        void highlightTemplateExpressions(const QString &text, int strStart, int strEnd);

        QSet<QString> keywords;
        QSet<QString> knownIds;
        const QHash<JavaScriptHighlighterPlugin::State, QTextCharFormat>* formats = nullptr;
};

JavaScriptSyntaxHighlighter::JavaScriptSyntaxHighlighter(QTextDocument *parent, const QHash<JavaScriptHighlighterPlugin::State, QTextCharFormat>* formats)
    : QSyntaxHighlighter(parent), formats(formats)
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

    int state = previousBlockState();
    setFormat(0, text.length(), formats->value(JavaScriptHighlighterPlugin::NORMAL));
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
                    setFormat(start, text.length(), formats->value(JavaScriptHighlighterPlugin::COMMENT));
                }
                else if (ch == '/' && next != '*')
                {
                    ++i;
                    state = Regex;
                }
                else
                {
                    if (!QString("(){}[]").contains(ch))
                        setFormat(start, 1, formats->value(JavaScriptHighlighterPlugin::NORMAL));
                    ++i;
                    state = Start;
                }
                break;

            case Number:
                if (ch.isSpace() || !ch.isDigit())
                {
                    setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::NUMBER));
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
                        setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::KEYWORDS));
                    else if (knownIds.contains(token))
                        setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::NORMAL));

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
                        setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::STRING));
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
                    setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::COMMENT));
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
                        setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::EXPRESSION));
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
        setFormat(start, text.length(), formats->value(JavaScriptHighlighterPlugin::COMMENT));
    else
        state = Start;

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
                    setFormat(start, i - start, formats->value(JavaScriptHighlighterPlugin::EXPRESSION));
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

bool JavaScriptHighlighterPlugin::init()
{
    refreshFormats();
    return true;
}

QString JavaScriptHighlighterPlugin::getLanguageName() const
{
    return QStringLiteral("JavaScript");
}

QSyntaxHighlighter* JavaScriptHighlighterPlugin::createSyntaxHighlighter(QWidget* textEdit) const
{
    QPlainTextEdit* plainEdit = dynamic_cast<QPlainTextEdit*>(textEdit);
    if (plainEdit)
        return new JavaScriptSyntaxHighlighter(plainEdit->document(), &formats);

    QTextEdit* edit = dynamic_cast<QTextEdit*>(textEdit);
    if (edit)
        return new JavaScriptSyntaxHighlighter(edit->document(), &formats);

    return nullptr;
}

void JavaScriptHighlighterPlugin::refreshFormats()
{
    QTextCharFormat format;

    format.setForeground(Cfg::getSyntaxForeground());
    formats[NORMAL] = format;

    format.setForeground(Cfg::getSyntaxNumberFg());
    formats[NUMBER] = format;

    format.setForeground(Cfg::getSyntaxKeywordFg());
    format.setFontWeight(QFont::Bold);
    formats[KEYWORDS] = format;

    format.setFontItalic(true);
    format.setFontWeight(QFont::Normal);
    format.setForeground(Cfg::getSyntaxCommentFg());
    formats[COMMENT] = format;

    format.setFontItalic(false);
    format.setForeground(Cfg::getSyntaxStringFg());
    formats[STRING] = format;

    format.setForeground(Cfg::getSyntaxNumberFg());
    formats[EXPRESSION] = format;
}

QString JavaScriptHighlighterPlugin::previewSampleCode() const
{
    static_qstring(code,
                   "function myFunction() { // Declare a function\n"
                   "    return \"Hello World!\";\n"
                   "}\n"
                   "\n"
                   "myFunction(); // Call the function"
                   );
    return code;
}
