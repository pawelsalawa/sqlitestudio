#ifndef CONSTRAINTTABMODEL_H
#define CONSTRAINTTABMODEL_H

#include "parser/ast/sqlitecreatetable.h"
#include <QAbstractTableModel>
#include <QPointer>

class ConstraintTabModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit ConstraintTabModel(QObject *parent = 0);

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;
        QVariant data(SqliteCreateTable::Constraint* constr, int column, int role) const;
        QVariant data(SqliteCreateTable::Column::Constraint* constr, int column, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

        void setCreateTable(const QPointer<SqliteCreateTable>& value);

    private:
        enum class Columns
        {
            SCOPE,
            TYPE,
            NAME,
            DETAILS
        };

        Columns getColumn(int idx) const;

        QString getTypeLabel(SqliteCreateTable::Constraint::Type type) const;
        QString getTypeLabel(SqliteCreateTable::Column::Constraint::Type type) const;

        QIcon getTypeIcon(SqliteCreateTable::Constraint::Type type) const;
        QIcon getTypeIcon(SqliteCreateTable::Column::Constraint::Type type) const;

        QString getDetails(SqliteCreateTable::Constraint* constr) const;
        QString getDetails(SqliteCreateTable::Column::Constraint* constr) const;

        QString getPkDetails(SqliteCreateTable::Constraint* constr) const;
        QString getUniqueDetails(SqliteCreateTable::Constraint* constr) const;
        QString getCheckDetails(SqliteCreateTable::Constraint* constr) const;
        QString getFkDetails(SqliteCreateTable::Constraint* constr) const;

        QString getPkDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getUniqueDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getCheckDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getFkDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getNotNullDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getCollateDetails(SqliteCreateTable::Column::Constraint* constr) const;
        QString getDefaultDetails(SqliteCreateTable::Column::Constraint* constr) const;

        QString getConstrDetails(SqliteCreateTable::Constraint* constr, int tokenOffset) const;
        QString getConstrDetails(SqliteCreateTable::Column::Constraint* constr, int tokenOffset) const;
        QString getConstrDetails(const TokenList& constrTokens, int tokenOffset) const;

        QPointer<SqliteCreateTable> createTable;

    signals:

    public slots:
        void updateModel();

};

#endif // CONSTRAINTTABMODEL_H
