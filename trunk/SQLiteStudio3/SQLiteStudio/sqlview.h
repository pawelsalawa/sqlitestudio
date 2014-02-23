#ifndef SQLVIEW_H
#define SQLVIEW_H

#include <QPlainTextEdit>

class SqliteSyntaxHighlighter;

class SqlView : public QPlainTextEdit
{
        Q_OBJECT
    public:
        explicit SqlView(QWidget *parent = 0);

        void setSqliteVersion(int version);

    private:
        SqliteSyntaxHighlighter* historyHighlighter;

    private slots:
        void changeFont(const QVariant& font);
};

#endif // SQLVIEW_H
