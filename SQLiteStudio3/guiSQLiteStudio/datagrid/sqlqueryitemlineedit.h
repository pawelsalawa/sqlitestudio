#ifndef SQLQUERYITEMLINEEDIT_H
#define SQLQUERYITEMLINEEDIT_H

#include "guiSQLiteStudio_global.h"
#include <QLineEdit>

class GUI_API_EXPORT SqlQueryItemLineEdit : public QLineEdit
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
