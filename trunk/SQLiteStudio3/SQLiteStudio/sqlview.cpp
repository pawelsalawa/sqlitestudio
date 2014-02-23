#include "sqlview.h"
#include "sqlitesyntaxhighlighter.h"
#include "uiconfig.h"

SqlView::SqlView(QWidget *parent) :
    QPlainTextEdit(parent)
{
    historyHighlighter = new SqliteSyntaxHighlighter(this->document());
    setFont(CFG_UI.Fonts.SqlEditor.get());
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
}

void SqlView::setSqliteVersion(int version)
{
    historyHighlighter->setSqliteVersion(version);
}

void SqlView::changeFont(const QVariant &font)
{
    setFont(font.value<QFont>());
}
