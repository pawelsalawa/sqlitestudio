#include "sqlview.h"
#include "sqlitesyntaxhighlighter.h"
#include "uiconfig.h"

SqlView::SqlView(QWidget *parent) :
    QTextEdit(parent)
{
    highlighter = new SqliteSyntaxHighlighter(this->document());
    setFont(CFG_UI.Fonts.SqlEditor.get());
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
    setReadOnly(true);
}

void SqlView::setSqliteVersion(int version)
{
    highlighter->setSqliteVersion(version);
}

SqliteSyntaxHighlighter* SqlView::getHighlighter() const
{
    return highlighter;
}

void SqlView::changeFont(const QVariant &font)
{
    setFont(font.value<QFont>());
}
