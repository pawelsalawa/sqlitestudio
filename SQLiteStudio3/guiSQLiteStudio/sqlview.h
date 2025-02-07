#ifndef SQLVIEW_H
#define SQLVIEW_H

#include "guiSQLiteStudio_global.h"
#include <QPlainTextEdit>

class SqliteSyntaxHighlighter;

class GUI_API_EXPORT SqlView : public QPlainTextEdit
{
        Q_OBJECT
    public:
        explicit SqlView(QWidget *parent = 0);

        void setTextBackgroundColor(int from, int to, const QColor& color);
        void setContents(const QString& value);

    private:
        SqliteSyntaxHighlighter* highlighter = nullptr;

    private slots:
        void changeFont(const QVariant& font);
};

#endif // SQLVIEW_H
