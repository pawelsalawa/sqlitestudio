#include "pythonsyntaxhighlighter.h"
#include "uiconfig.h"
#include "services/config.h"
#include <QtWidgets/QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

// Started from Qt Syntax Highlighter example and then ported https://wiki.python.org/moin/PyQt/Python%20syntax%20highlighting
// Ported code copied from https://forum.qt.io/topic/96285/c-highlighter-for-python
// and then adjusted for SQLiteStudio (i.e. migrated from QRegExp to QRegularExpression).
class PythonHighlighter : public QSyntaxHighlighter
{
    public:
        typedef QMap<QString, QTextCharFormat> FormatMap;

        PythonHighlighter(QTextDocument *parent, const QMap<PythonSyntaxHighlighterPlugin::State, QTextCharFormat>* styles);

    protected:
        void highlightBlock(const QString &text) override;

    private:
        struct HighlightingRule
        {
            QRegularExpression pattern;
            PythonSyntaxHighlighterPlugin::State state;
            int matchIndex = 0;

            HighlightingRule() { }
            HighlightingRule(const QRegularExpression &r, int i, PythonSyntaxHighlighterPlugin::State state) : pattern(r), state(state), matchIndex(i) { }
            HighlightingRule(const QString &p, int i, PythonSyntaxHighlighterPlugin::State state) : pattern(QRegularExpression(p)), state(state), matchIndex(i) { }
        };

        const QMap<PythonSyntaxHighlighterPlugin::State, QTextCharFormat>* styles;
        static const QStringList keywords;
        static const QStringList operators;
        static const QStringList braces;

        void initialize();
        void highlightPythonBlock(const QString &text);
        bool matchMultiLine(const QString &text, const HighlightingRule &rule);

        QVector<HighlightingRule> _pythonHighlightingRules;
        HighlightingRule _triSingle, _triDouble;
};

// Python keywords
const QStringList PythonHighlighter::keywords = {
    "and", "assert", "break", "class", "continue", "def",
    "del", "elif", "else", "except", "exec", "finally",
    "for", "from", "global", "if", "import", "in",
    "is", "lambda", "not", "or", "pass", "print",
    "raise", "return", "try", "while", "yield",
    "None", "True", "False"
};

// Python operators
const QStringList PythonHighlighter::operators = {
    "\\=",
    // Comparison
    "\\=\\=", "\\!\\=", "\\<", "\\<\\=", "\\>", "\\>\\=",
    // Arithmetic
    "\\+", "\\-", "\\*", "\\/", "\\/\\/", "\\%", "\\*\\*",
    // In-place
    "\\+\\=", "\\-\\=", "\\*\\=", "\\/\\=", "\\%\\=",
    // Bitwise
    "\\^", "\\|", "\\&", "\\~", "\\>\\>", "\\<\\<"
};

// Python braces
const QStringList PythonHighlighter::braces = {
    "\\{", "\\}", "\\(", "\\)", "\\[", "\\]"
};

PythonHighlighter::PythonHighlighter(QTextDocument *parent, const QMap<PythonSyntaxHighlighterPlugin::State, QTextCharFormat>* styles)
    : QSyntaxHighlighter(parent), styles(styles)
{
    initialize();
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    highlightPythonBlock(text);
}

void PythonHighlighter::initialize()
{
    // Multi-line strings (expression, flag, style)
    // FIXME: The triple-quotes in these two lines will mess up the
    // syntax highlighting from this point onward
    _triSingle = HighlightingRule("'''", 1, PythonSyntaxHighlighterPlugin::STRING);
    _triDouble = HighlightingRule("\"\"\"", 2, PythonSyntaxHighlighterPlugin::STRING);

    // Keyword, operator, and brace rules
    for (const QString &keyword : keywords)
    {
        QString pattern = QString("\\b%1\\b").arg(keyword);
        _pythonHighlightingRules += HighlightingRule(pattern, 0, PythonSyntaxHighlighterPlugin::KEYWORD);
    }

    for (const QString &pattern: operators)
        _pythonHighlightingRules += HighlightingRule(pattern, 0, PythonSyntaxHighlighterPlugin::OPERATOR);

    for (const QString &pattern: braces)
        _pythonHighlightingRules += HighlightingRule(pattern, 0, PythonSyntaxHighlighterPlugin::BRACE);

    // All other rules

    // 'self'
    _pythonHighlightingRules += HighlightingRule("\\bself\\b", 0, PythonSyntaxHighlighterPlugin::SELF);

    // Double-quoted string, possibly containing escape sequences
    _pythonHighlightingRules += HighlightingRule("\"([^\"\\\\]|\\\\.)*\"", 0, PythonSyntaxHighlighterPlugin::STRING);
    // Single-quoted string, possibly containing escape sequences
    _pythonHighlightingRules += HighlightingRule("'([^'\\\\]|\\\\.)*'", 0, PythonSyntaxHighlighterPlugin::STRING);

    // 'def' followed by an identifier
    _pythonHighlightingRules += HighlightingRule("\\bdef\\b\\s*(\\w+)", 1, PythonSyntaxHighlighterPlugin::DEFCLASS);
    // 'class' followed by an identifier
    _pythonHighlightingRules += HighlightingRule("\\bclass\\b\\s*(\\w+)", 1, PythonSyntaxHighlighterPlugin::DEFCLASS);

    // From '#' until a newline
    _pythonHighlightingRules += HighlightingRule("#[^\\n]*", 0, PythonSyntaxHighlighterPlugin::COMMENT);

    // Numeric literals
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?[0-9]+[lL]?\\b", 0, PythonSyntaxHighlighterPlugin::NUMBER);
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0, PythonSyntaxHighlighterPlugin::NUMBER);
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0, PythonSyntaxHighlighterPlugin::NUMBER);
}

void PythonHighlighter::highlightPythonBlock(const QString &text)
{
    if (text.isEmpty())
        return;

    int index = -1;
    setFormat(0, text.length(), styles->value(PythonSyntaxHighlighterPlugin::STANDARD));

    // Do other syntax formatting
    for (HighlightingRule& rule : _pythonHighlightingRules)
    {
        QRegularExpressionMatchIterator iter = rule.pattern.globalMatch(text, 0);
        while (iter.hasNext())
        {
            QRegularExpressionMatch match = iter.next();
            index = match.capturedStart(rule.matchIndex);
            int length = match.capturedLength(rule.matchIndex);
            if (length > 0)
                setFormat(index, length, styles->value(rule.state));
        }
    }

    setCurrentBlockState(0);

    // Do multi-line strings
    bool in_multiline = matchMultiLine(text, _triSingle);
    if (!in_multiline)
        matchMultiLine(text, _triDouble);
}

/*Do highlighting of multi-line strings. ``delimiter`` should be a
``QRegExp`` for triple-single-quotes or triple-double-quotes, and
``in_state`` should be a unique integer to represent the corresponding
state changes when inside those strings. Returns True if we're still
inside a multi-line string when this function is finished.
*/
bool PythonHighlighter::matchMultiLine(const QString &text, const HighlightingRule &rule)
{
    int start, add, end, length;

    // If inside triple-single quotes, start at 0
    if (previousBlockState() == rule.matchIndex)
    {
        start = 0;
        add = 0;
    }
    // Otherwise, look for the delimiter on this line
    else
    {
        QRegularExpressionMatch match = rule.pattern.match(text);
        start = match.capturedStart();
        // Move past this match
        add = match.capturedLength();
    }

    // As long as there's a delimiter match on this line...
    while (start >= 0)
    {
        QRegularExpressionMatch match = rule.pattern.match(text, start + add);
        // Look for the ending delimiter
        end = match.capturedStart();
        // Ending delimiter on this line?
        if(end >= add)
        {
            length = end - start + add + match.capturedLength();
            setCurrentBlockState(0);
        }
        // No; multi-line string
        else
        {
            setCurrentBlockState(rule.matchIndex);
            length = text.length() - start + add;
        }

        // Apply formatting
        setFormat(start, length, styles->value(rule.state));

        // Look for the next match
        match = rule.pattern.match(text, start + length);
        start = match.capturedStart();
    }

    // Return True if still inside a multi-line string, False otherwise
    if (currentBlockState() == rule.matchIndex)
        return true;
    else
        return false;
}


PythonSyntaxHighlighterPlugin::PythonSyntaxHighlighterPlugin()
{
}

QString PythonSyntaxHighlighterPlugin::getLanguageName() const
{
    return QStringLiteral("Python");
}

QSyntaxHighlighter* PythonSyntaxHighlighterPlugin::createSyntaxHighlighter(QWidget* textEdit) const
{
    QPlainTextEdit* plainEdit = dynamic_cast<QPlainTextEdit*>(textEdit);
    if (plainEdit)
        return new PythonHighlighter(plainEdit->document(), &styles);

    QTextEdit* edit = dynamic_cast<QTextEdit*>(textEdit);
    if (edit)
        return new PythonHighlighter(edit->document(), &styles);

    return nullptr;
}

void PythonSyntaxHighlighterPlugin::refreshFormats()
{
    QTextCharFormat format;

    // Standard
    format.setForeground(Cfg::getSyntaxForeground());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    styles[PythonSyntaxHighlighterPlugin::STANDARD] = format;

    // Class
    format.setForeground(Cfg::getSyntaxKeywordFg());
    format.setFontWeight(QFont::Bold);
    styles[PythonSyntaxHighlighterPlugin::DEFCLASS] = format;
    styles[PythonSyntaxHighlighterPlugin::KEYWORD] = format;

    // Self
    format.setForeground(Cfg::getSyntaxKeywordFg());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(true);
    styles[PythonSyntaxHighlighterPlugin::SELF] = format;

    // Operator
    format.setForeground(Cfg::getSyntaxForeground());
    format.setFontItalic(false);
    styles[PythonSyntaxHighlighterPlugin::OPERATOR] = format;

    // Parenthesis
    format.setForeground(Cfg::getSyntaxForeground());
    styles[PythonSyntaxHighlighterPlugin::BRACE] = format;

    // String
    format.setForeground(Cfg::getSyntaxStringFg());
    styles[PythonSyntaxHighlighterPlugin::STRING] = format;

    // Numbers
    format.setForeground(Cfg::getSyntaxNumberFg());
    styles[PythonSyntaxHighlighterPlugin::NUMBER] = format;

    // Comment
    format.setForeground(Cfg::getSyntaxCommentFg());
    format.setFontItalic(true);
    styles[PythonSyntaxHighlighterPlugin::COMMENT] = format;
}

QString PythonSyntaxHighlighterPlugin::previewSampleCode() const
{
    static_qstring(code,
                   "class MyClass:\n"
                   "    \"\"\"A simple example class\"\"\"\n"
                   "    i = 12345\n"
                   "\n"
                   "    def f(self):\n"
                   "        return 'hello world'"""
                   );
    return code;
}
