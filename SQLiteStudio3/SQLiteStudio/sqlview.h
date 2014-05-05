#ifndef SQLVIEW_H
#define SQLVIEW_H

#include <QTextEdit>

class SqliteSyntaxHighlighter;

class SqlView : public QTextEdit
{
        Q_OBJECT
    public:
        explicit SqlView(QWidget *parent = 0);

        void setSqliteVersion(int version);
        SqliteSyntaxHighlighter* getHighlighter() const;

    private:
        SqliteSyntaxHighlighter* highlighter;

    private slots:
        void changeFont(const QVariant& font);
};

#endif // SQLVIEW_H
