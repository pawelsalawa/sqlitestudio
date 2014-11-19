#ifndef SQLVIEW_H
#define SQLVIEW_H

#include "guiSQLiteStudio_global.h"
#include <QTextEdit>

class SqliteSyntaxHighlighter;

class GUI_API_EXPORT SqlView : public QTextEdit
{
        Q_OBJECT
    public:
        explicit SqlView(QWidget *parent = 0);

        void setSqliteVersion(int version);
        void setTextBackgroundColor(int from, int to, const QColor& color);

    private:
        SqliteSyntaxHighlighter* highlighter = nullptr;

    private slots:
        void changeFont(const QVariant& font);
};

#endif // SQLVIEW_H
