#ifndef SYNTAXHIGHLIGHTERPLUGIN_H
#define SYNTAXHIGHLIGHTERPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugins/plugin.h"

class QWidget;
class QSyntaxHighlighter;

class SyntaxHighlighterPlugin : virtual public Plugin
{
    public:
        virtual QString getLanguageName() const = 0;
        virtual QSyntaxHighlighter* createSyntaxHighlighter(QWidget* textEdit) const = 0;
};

#endif // SYNTAXHIGHLIGHTERPLUGIN_H
