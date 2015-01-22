#ifndef COLUMNDEFAULTPANEL_H
#define COLUMNDEFAULTPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqliteconflictalgo.h"
#include "guiSQLiteStudio_global.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QWidget>

namespace Ui {
    class ColumnDefaultPanel;
}

class GUI_API_EXPORT ColumnDefaultPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnDefaultPanel(QWidget *parent = 0);
        ~ColumnDefaultPanel();

        bool validate();
        bool validateOnly();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        enum class Mode
        {
            EXPR,
            LITERAL,
            ERROR
        };

        void init();
        void readConstraint();
        void updateVirtualSql();
        QString getTempTable();
        void storeExpr(SqliteCreateTable::Column::Constraint* constr);
        void storeLiteral(SqliteCreateTable::Column::Constraint* constr);
        void clearDefault(SqliteCreateTable::Column::Constraint* constr);

        Ui::ColumnDefaultPanel *ui = nullptr;
        QString lastValidatedText;
        bool lastValidationResult = false;
        Mode currentMode = Mode::ERROR;

    private slots:
        void updateState();
};

#endif // COLUMNDEFAULTPANEL_H
