#ifndef PYTHONSYNTAXHIGHLIGHTER_H
#define PYTHONSYNTAXHIGHLIGHTER_H

#include "pythonsyntaxhighlighter_global.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/genericplugin.h"
#include <QMap>
#include <QObject>
#include <QString>
#include <QTextCharFormat>

class PYTHONSYNTAXHIGHLIGHTERSHARED_EXPORT PythonSyntaxHighlighterPlugin : public GenericPlugin, public SyntaxHighlighterPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pythonsyntaxhighlighter.json")

    public:
        enum State
        {
            STANDARD,
            KEYWORD,
            DEFCLASS,
            SELF,
            OPERATOR,
            BRACE,
            STRING,
            NUMBER,
            COMMENT
        };

        PythonSyntaxHighlighterPlugin();
		
        QString getLanguageName() const;
        QSyntaxHighlighter* createSyntaxHighlighter(QWidget* textEdit) const;
        QString previewSampleCode() const;
        void refreshFormats();

    private:
        QMap<PythonSyntaxHighlighterPlugin::State, QTextCharFormat> styles;
};

#endif // PYTHONSYNTAXHIGHLIGHTER_H
