#ifndef COLUMNDIALOGCONSTRAINTSMODEL_H
#define COLUMNDIALOGCONSTRAINTSMODEL_H

#include "parser/ast/sqlitecreatetable.h"
#include <QAbstractTableModel>
#include <QPointer>

class ColumnDialogConstraintsModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit ColumnDialogConstraintsModel(QObject *parent = 0);

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        int columnCount(const QModelIndex& parent) const;
        QVariant data(const QModelIndex& index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        void setColumn(SqliteCreateTable::Column* value);
        SqliteCreateTable::Column::Constraint* getConstraint(int constrIdx) const;
        void replaceConstraint(int constrIdx, SqliteCreateTable::Column::Constraint* constr);
        void insertConstraint(int constrIdx, SqliteCreateTable::Column::Constraint* constr);
        void appendConstraint(SqliteCreateTable::Column::Constraint* constr);
        void delConstraint(int constrIdx);
        void delConstraint(SqliteCreateTable::Column::Constraint* constr);
        void moveConstraintUp(int constrIdx);
        void moveConstraintDown(int constrIdx);
        void moveConstraintColumnTo(int constrIdx, int newIdx);

    private:
        enum class Column
        {
            TYPE,
            NAME,
            DETAILS
        };

        Column getColumn(int colIdx) const;
        QIcon getIcon(int rowIdx) const;
        QString getName(int rowIdx) const;
        QString getType(int rowIdx) const;
        QString getDetails(int rowIdx) const;
        QString getPkDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getNotNullDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getUniqueDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getCheckDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getDefaultDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getCollateDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getFkDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getConstrDetails(SqliteCreateTable::Column::Constraint* constr, int tokenOffset) const;

        QPointer<SqliteCreateTable::Column> column;

    signals:
        void constraintsChanged();
};

#endif // COLUMNDIALOGCONSTRAINTSMODEL_H
