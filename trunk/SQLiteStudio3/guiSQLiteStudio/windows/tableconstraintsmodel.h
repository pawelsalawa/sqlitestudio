#ifndef TABLECONSTRAINTSMODEL_H
#define TABLECONSTRAINTSMODEL_H

#include "parser/ast/sqlitecreatetable.h"
#include "guiSQLiteStudio_global.h"
#include <QAbstractTableModel>
#include <QPointer>

class GUI_API_EXPORT TableConstraintsModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit TableConstraintsModel(QObject *parent = 0);

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        Qt::DropActions supportedDropActions() const;
        Qt::DropActions supportedDragActions() const;
        QStringList mimeTypes() const;
        QMimeData* mimeData(const QModelIndexList& indexes) const;
        bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
        Qt::ItemFlags flags(const QModelIndex& index) const;
        bool isModified() const;
        void setCreateTable(SqliteCreateTable* value);
        SqliteCreateTable::Constraint* getConstraint(int constrIdx) const;
        void replaceConstraint(int constrIdx, SqliteCreateTable::Constraint* constr);
        void constraintModified(int constrIdx);
        void insertConstraint(int constrIdx, SqliteCreateTable::Constraint* constr);
        void appendConstraint(SqliteCreateTable::Constraint* constr);
        void delConstraint(int constrIdx);
        void moveConstraintUp(int constrIdx);
        void moveConstraintDown(int constrIdx);
        void moveConstraintColumnTo(int constrIdx, int newIdx);

    private:
        enum class Columns
        {
            TYPE,
            NAME,
            DETAILS
        };

        Columns getColumn(int idx) const;
        QString getTypeLabel(SqliteCreateTable::Constraint::Type type) const;
        QIcon getTypeIcon(SqliteCreateTable::Constraint::Type type) const;
        QString getDetails(SqliteCreateTable::Constraint* constr) const;
        QString getPkDetails(SqliteCreateTable::Constraint* constr) const;
        QString getUniqueDetails(SqliteCreateTable::Constraint* constr) const;
        QString getCheckDetails(SqliteCreateTable::Constraint* constr) const;
        QString getFkDetails(SqliteCreateTable::Constraint* constr) const;
        QString getConstrDetails(SqliteCreateTable::Constraint* constr, int tokenOffset) const;
        void columnRenamed(SqliteCreateTable::Constraint* constr, const QString& oldColumn, const QString& newColumn);
        bool handleColumnDeleted(SqliteCreateTable::Constraint* constr, const QString& column);

        static const constexpr char* mimeType = "application/x-sqlitestudio-tablestructureconstraintmodel-row-index";

        QPointer<SqliteCreateTable> createTable;
        bool modified = false;

    public slots:
        void columnModified(const QString& oldColumn, SqliteCreateTable::Column* newColumn);
        void columnDeleted(const QString& column);

    signals:
        void modifiyStateChanged();
        void constraintOrderChanged();
};

#endif // TABLECONSTRAINTSMODEL_H
