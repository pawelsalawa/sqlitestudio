#ifndef TCLSYNTAXHIGHLIGHTER_H
#define TCLSYNTAXHIGHLIGHTER_H

#include "TclSyntaxHighlighter_global.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/genericplugin.h"
#include <QObject>
#include <QTextCharFormat>
#include <QRegularExpression>

class TCLSYNTAXHIGHLIGHTER_EXPORT TclSyntaxHighlighterPlugin : public GenericPlugin, public SyntaxHighlighterPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("tclsyntaxhighlighter.json")

    public:
        enum State
        {
            STANDARD,
            KEYWORD,
            BUILTIN,
            VARIABLE,
            OPERATOR,
            BRACE,
            STRING,
            NUMBER,
            COMMENT
        };

        struct Rule
        {
            QRegularExpression pattern;
            State state;
        };

        TclSyntaxHighlighterPlugin();

        QString getLanguageName() const;
        QSyntaxHighlighter* createSyntaxHighlighter(QWidget* textEdit) const;
        QString previewSampleCode() const;
        void refreshFormats();

    private:
        void initRules();

        QVector<Rule> rules;
        QMap<State, QTextCharFormat> styles;
};

#endif // TCLSYNTAXHIGHLIGHTER_H
