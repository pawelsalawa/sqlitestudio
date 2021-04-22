#ifndef PYTHONSYNTAXHIGHLIGHTER_H
#define PYTHONSYNTAXHIGHLIGHTER_H

#include "pythonsyntaxhighlighter_global.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/genericplugin.h"
#include <QObject>

class PYTHONSYNTAXHIGHLIGHTERSHARED_EXPORT PythonSyntaxHighlighterPlugin : public GenericPlugin, public SyntaxHighlighterPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pythonsyntaxhighlighter.json")

    public:
        PythonSyntaxHighlighterPlugin();
		
        QString getLanguageName() const;
        QSyntaxHighlighter* createSyntaxHighlighter(QWidget* textEdit) const;
};

#endif // PYTHONSYNTAXHIGHLIGHTER_H
