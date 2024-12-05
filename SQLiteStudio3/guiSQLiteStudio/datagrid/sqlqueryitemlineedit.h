#ifndef SQLQUERYITEMLINEEDIT_H
#define SQLQUERYITEMLINEEDIT_H

#include <QLineEdit>

class SqlQueryItemLineEdit : public QLineEdit
{
    Q_OBJECT

    public:
        explicit SqlQueryItemLineEdit(bool shouldSkipInitialSelection, QWidget *parent = nullptr);

    protected:
        void focusInEvent(QFocusEvent *e);

    private:
        bool shouldSkipInitialSelection;
};

#endif // SQLQUERYITEMLINEEDIT_H
