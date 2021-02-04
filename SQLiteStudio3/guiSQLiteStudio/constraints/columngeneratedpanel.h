#ifndef COLUMNGENERATEDPANEL_H
#define COLUMNGENERATEDPANEL_H

#include "constraintpanel.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>

namespace Ui {
    class ColumnGeneratedPanel;
}

class GUI_API_EXPORT ColumnGeneratedPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnGeneratedPanel(QWidget *parent = 0);
        ~ColumnGeneratedPanel();

        bool validate();
        bool validateOnly();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        void init();
        void readConstraint();
        void updateVirtualSql();
        QString getTempTable();
        void storeExpr(SqliteCreateTable::Column::Constraint* constr);
        void clear(SqliteCreateTable::Column::Constraint* constr);

        Ui::ColumnGeneratedPanel *ui = nullptr;
        QString lastValidatedText;
        bool lastValidationResult = false;

    private slots:
        void updateState();
};

#endif // COLUMNGENERATEDPANEL_H
