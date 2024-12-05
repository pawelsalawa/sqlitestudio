#ifndef COLUMNDIALOG_H
#define COLUMNDIALOG_H

#include "parser/ast/sqlitecreatetable.h"
#include "common/extactioncontainer.h"
#include "constraintdialog.h"
#include "guiSQLiteStudio_global.h"
#include "common/strhash.h"
#include <QDialog>
#include <QPointer>

class ColumnDialogConstraintsModel;
class QCheckBox;
class QToolButton;

namespace Ui {
    class ColumnDialog;
}

class GUI_API_EXPORT ColumnDialog : public QDialog, public ExtActionContainer
{
        Q_OBJECT

    public:
        enum Action
        {
            ADD_CONSTRAINT,
            EDIT_CONSTRAINT,
            DEL_CONSTRAINT,
            MOVE_CONSTRAINT_UP,
            MOVE_CONSTRAINT_DOWN,
            ADD_PK,
            ADD_FK,
            ADD_UNIQUE,
            ADD_CHECK,
            ADD_DEFAULT,
            ADD_NOT_NULL,
            ADD_GENERATED,
            ADD_COLLATE
        };

        enum ToolBar
        {
        };

        explicit ColumnDialog(Db* db, QWidget *parent = 0);
        ~ColumnDialog();

        void init();
        void setColumn(SqliteCreateTable::Column* value);
        SqliteCreateTable::Column* getModifiedColumn();
        QToolBar* getToolBar(int toolbar) const;
        void disableConstraint(ConstraintDialog::Constraint constraint);

    protected:
        void changeEvent(QEvent *e);
        void createActions();
        void setupDefShortcuts();

    private:
        void addConstraint(ConstraintDialog::Constraint mode);
        void setupConstraintCheckBoxes();
        void editConstraint(SqliteCreateTable::Column::Constraint* constraint);
        void delConstraint(const QModelIndex& idx);
        void configureConstraint(SqliteCreateTable::Column::Constraint::Type type);
        void addEmptyConstraint(SqliteCreateTable::Column::Constraint::Type type);
        void delAllConstraint(SqliteCreateTable::Column::Constraint::Type type);
        void constraintToggled(SqliteCreateTable::Column::Constraint::Type type, bool enabled);
        void updateConstraintState(SqliteCreateTable::Column::Constraint* constraint);
        QCheckBox* getCheckBoxForConstraint(SqliteCreateTable::Column::Constraint* constraint);
        QToolButton* getToolButtonForConstraint(SqliteCreateTable::Column::Constraint* constraint);
        void updateTypeValidations();
        void updateTypeForAutoIncr();
        bool hasAutoIncr() const;
        void validateFkTypeMatch();

        Ui::ColumnDialog *ui = nullptr;
        SqliteCreateTable::ColumnPtr column;
        ColumnDialogConstraintsModel* constraintsModel = nullptr;
        QCheckBox* modeCheckBox = nullptr;
        Db* db = nullptr;
        bool integerTypeEnforced = false;
        QSet<ConstraintDialog::Constraint> disabledConstraints;
        StrHash<StrHash<DataType>> fkTableTypesCache;

    private slots:
        void updateConstraintsToolbarState();
        void updateState();
        void addConstraint();
        void editConstraint();
        void editConstraint(const QModelIndex& idx);
        void delConstraint();
        void moveConstraintUp();
        void moveConstraintDown();
        void addPk();
        void addFk();
        void addUnique();
        void addCheck();
        void addCollate();
        void addNotNull();
        void addGenerated();
        void addDefault();
        void configurePk();
        void configureFk();
        void configureUnique();
        void configureCheck();
        void configureCollate();
        void configureGenerated();
        void configureNotNull();
        void configureDefault();
        void pkToggled(bool enabled);
        void fkToggled(bool enabled);
        void uniqueToggled(bool enabled);
        void checkToggled(bool enabled);
        void collateToggled(bool enabled);
        void generatedToggled(bool enabled);
        void notNullToggled(bool enabled);
        void defaultToggled(bool enabled);
        void switchMode(bool advanced);
        void updateValidations();
        void updateConstraint(SqliteCreateTable::Column::Constraint* constraint);
        void updateDataType();
};

#endif // COLUMNDIALOG_H
