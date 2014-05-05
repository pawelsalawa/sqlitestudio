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

void SqlView::setTextBackgroundColor(int from, int to, const QColor& color)
{
    bool wasRo = false;
    if (isReadOnly())
    {
        wasRo = true;
        setReadOnly(false);
    }

    QTextCharFormat format;
    format.setBackground(color);

    QTextCursor cur(document());
    cur.setPosition(from);
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, to - from + 1);
    cur.mergeCharFormat(format);

    if (wasRo)
        setReadOnly(true);
}

void SqlView::changeFont(const QVariant &font)
{
    setFont(font.value<QFont>());
}
