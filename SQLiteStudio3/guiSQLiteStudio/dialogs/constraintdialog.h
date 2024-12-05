#ifndef CONSTRAINTDIALOG_H
#define CONSTRAINTDIALOG_H

#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QPointer>
#include <QHash>

namespace Ui {
    class ConstraintDialog;
}

class ConstraintPanel;

class GUI_API_EXPORT ConstraintDialog : public QDialog
{
        Q_OBJECT

    public:
        enum Mode
        {
            NEW,
            EDIT
        };

        enum Constraint
        {
            PK,
            FK,
            UNIQUE,
            NOTNULL,
            CHECK,
            COLLATE,
            GENERATED,
            DEFAULT,
            UNKNOWN
        };

        enum Type
        {
            TABLE,
            COLUMN
        };

        explicit ConstraintDialog(Mode mode, SqliteCreateTable::Constraint* constraint, SqliteCreateTable* createTable, Db* db,
                                  QWidget *parent = 0);
        explicit ConstraintDialog(Mode mode, SqliteCreateTable::Column::Constraint* constraint, SqliteCreateTable::Column* column, Db* db,
                                  QWidget *parent = 0);
        ~ConstraintDialog();

        SqliteStatement* getConstraint();

    protected:
        void changeEvent(QEvent *e);

    private:
        void init();
        Constraint getSelectedConstraint();
        Constraint getSelectedConstraint(SqliteCreateTable::Constraint* constraint);
        Constraint getSelectedConstraint(SqliteCreateTable::Column::Constraint* constraint);
        ConstraintPanel* createConstraintPanel();
        void updateDefinitionHeader();

        Ui::ConstraintDialog *ui = nullptr;
        Type type;
        Mode mode;
        Db* db = nullptr;
        SqliteStatement* constrStatement = nullptr;
        QPointer<SqliteCreateTable> createTable;
        QPointer<SqliteCreateTable::Column> columnStmt;
        QHash<int,QWidget> panels;
        ConstraintPanel* currentPanel = nullptr;

    private slots:
        void validate();
        void storeConfiguration();
};

#endif // CONSTRAINTDIALOG_H
