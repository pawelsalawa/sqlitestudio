#include "tclsyntaxhighlighter.h"
#include "uiconfig.h"
#include "services/config.h"
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

class TclSyntaxHighlighter: public QSyntaxHighlighter
{
    public:
        TclSyntaxHighlighter(QTextDocument *parent,
                          const QMap<TclSyntaxHighlighterPlugin::State, QTextCharFormat>& styles,
                          const QVector<TclSyntaxHighlighterPlugin::Rule>& rules);

    protected:
        void highlightBlock(const QString &text) override;

    private:
        const QMap<TclSyntaxHighlighterPlugin::State, QTextCharFormat>& styles;
        const QVector<TclSyntaxHighlighterPlugin::Rule>& rules;
};

TclSyntaxHighlighter::TclSyntaxHighlighter(QTextDocument* parent,
                                        const QMap<TclSyntaxHighlighterPlugin::State,
                                        QTextCharFormat>& styles, const QVector<TclSyntaxHighlighterPlugin::Rule>& rules) :
    QSyntaxHighlighter(parent), styles(styles), rules(rules)
{

}

void TclSyntaxHighlighter::highlightBlock(const QString& text)
{
    setFormat(0, text.length(), styles[TclSyntaxHighlighterPlugin::STANDARD]);

    for (const TclSyntaxHighlighterPlugin::Rule &rule : std::as_const(rules))
    {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext())
        {
            const auto match = it.next();
            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                styles[rule.state]
            );
        }
    }
}

TclSyntaxHighlighterPlugin::TclSyntaxHighlighterPlugin()
{
    initRules();
}

QString TclSyntaxHighlighterPlugin::getLanguageName() const
{
    return QStringLiteral("Tcl");
}

QSyntaxHighlighter* TclSyntaxHighlighterPlugin::createSyntaxHighlighter(QWidget* textEdit) const
{
    QPlainTextEdit* plainEdit = dynamic_cast<QPlainTextEdit*>(textEdit);
    if (plainEdit)
        return new TclSyntaxHighlighter(plainEdit->document(), styles, rules);

    QTextEdit* edit = dynamic_cast<QTextEdit*>(textEdit);
    if (edit)
        return new TclSyntaxHighlighter(edit->document(), styles, rules);

    return nullptr;
}

QString TclSyntaxHighlighterPlugin::previewSampleCode() const
{
    static_qstring(code,
       "proc factorial {n} {\n"
       "    if {$n <= 1} {\n"
       "        return 1\n"
       "    }\n"
       "\n"
       "    set result 1\n"
       "    foreach i {1 2 3 4 5} {\n"
       "        incr result $i\n"
       "        puts \"intermediate result = $result\"\n"
       "    }\n"
       "\n"
       "    return [expr {$result * $n}]\n"
       "}\n"
       "\n"
       "# Run\n"
       "puts [factorial 5]"
    );
    return code;
}

void TclSyntaxHighlighterPlugin::refreshFormats()
{
    styles[STANDARD] = Cfg::getSyntaxForegroundFormat();
    styles[KEYWORD] = Cfg::getSyntaxKeywordFormat();
    styles[BUILTIN] = Cfg::getSyntaxKeywordFormat();
    styles[VARIABLE] = Cfg::getSyntaxBindParamFormat();
    styles[OPERATOR] = Cfg::getSyntaxForegroundFormat();
    styles[BRACE] = Cfg::getSyntaxForegroundFormat();
    styles[STRING] = Cfg::getSyntaxStringFormat();
    styles[NUMBER] = Cfg::getSyntaxNumberFormat();
    styles[COMMENT] = Cfg::getSyntaxCommentFormat();
}

void TclSyntaxHighlighterPlugin::initRules()
{
    const QStringList keywords = {
        "if", "then", "else", "elseif",
        "for", "foreach", "while",
        "break", "continue", "return",
        "switch", "default",
        "proc", "namespace",
        "try", "catch", "finally",
        "eval", "uplevel"
    };

    for (const QString &kw : keywords) {
        rules.append({
            QRegularExpression(QStringLiteral("\\b%1\\b").arg(kw)),
            KEYWORD
        });
    }

    const QStringList builtins = {
        "set", "unset", "append", "lappend", "incr",
        "puts", "gets", "expr", "info", "rename",
        "source", "format", "scan", "subst",
        "string", "list", "array", "dict"
    };

    for (const QString &kw : builtins)
    {
        rules.append({
            QRegularExpression(QStringLiteral("\\b%1\\b").arg(kw)),
            BUILTIN
        });
    }

    rules.append({
        QRegularExpression(R"(\$(?:\{[^}]+\}|::)?[A-Za-z_][A-Za-z0-9_:]*)"),
        VARIABLE
    });

    rules.append({
        QRegularExpression(R"(\b\d+(\.\d+)?\b)"),
        NUMBER
    });

    rules.append({
        QRegularExpression(R"("([^"\\]|\\.)*")"),
        STRING
    });

    rules.append({
        QRegularExpression(R"(#.*$)"),
        COMMENT
    });

    rules.append({
        QRegularExpression(R"(==|!=|<=|>=|\+|-|\*|/|<|>|\|\||&&)"),
        OPERATOR
    });

    rules.append({
        QRegularExpression(R"([\{\}])"),
        BRACE
    });}
