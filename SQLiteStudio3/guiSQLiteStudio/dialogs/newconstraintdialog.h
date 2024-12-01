#ifndef NEWCONSTRAINTDIALOG_H
#define NEWCONSTRAINTDIALOG_H

#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "dialogs/constraintdialog.h"
#include "iconmanager.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QPointer>
#include <QHash>

namespace Ui {
    class NewConstraintDialog;
}
class QCommandLinkButton;

class GUI_API_EXPORT NewConstraintDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit NewConstraintDialog(SqliteCreateTable* createTable, Db* db, QWidget *parent = 0);
        explicit NewConstraintDialog(SqliteCreateTable::Column* column, Db* db, QWidget *parent = 0);
        explicit NewConstraintDialog(ConstraintDialog::Constraint constraintType, SqliteCreateTable* createTable, Db* db, QWidget *parent = 0);
        explicit NewConstraintDialog(ConstraintDialog::Constraint constraintType, SqliteCreateTable::Column* column, Db* db, QWidget *parent = 0);
        ~NewConstraintDialog();

        SqliteStatement* getConstraint();
        void disableMode(ConstraintDialog::Constraint constraintType);
        int exec();

    protected:
        void changeEvent(QEvent *e);

    private:
        void init();
        void initTable();
        void initColumn();
        QCommandLinkButton* addButton(const Icon& icon, const QString text, const char* slot);
        int createColumnConstraint(ConstraintDialog::Constraint constraintType);
        int createTableConstraint(ConstraintDialog::Constraint constraintType);
        int editConstraint();

        Ui::NewConstraintDialog *ui = nullptr;
        ConstraintDialog::Type type;
        Db* db = nullptr;
        ConstraintDialog::Constraint predefinedConstraintType = ConstraintDialog::UNKNOWN;
        SqliteStatement* constrStatement = nullptr;
        QPointer<SqliteCreateTable> createTable;
        QPointer<SqliteCreateTable::Column> columnStmt;
        ConstraintDialog* constraintDialog = nullptr;
        QHash<ConstraintDialog::Constraint, QCommandLinkButton*> modeToButton;

    private slots:
        void createTablePk();
        void createTableFk();
        void createTableUnique();
        void createTableCheck();
        void createColumnPk();
        void createColumnFk();
        void createColumnUnique();
        void createColumnCheck();
        void createColumnNotNull();
        void createColumnDefault();
        void createColumnCollate();
        void createColumnGenerated();
};

#endif // NEWCONSTRAINTDIALOG_H
