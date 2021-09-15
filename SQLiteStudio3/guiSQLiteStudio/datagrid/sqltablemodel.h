#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include "guiSQLiteStudio_global.h"
#include "sqldatasourcequerymodel.h"

class GUI_API_EXPORT SqlTableModel : public SqlDataSourceQueryModel
{
        Q_OBJECT
    public:
        explicit SqlTableModel(QObject *parent = 0);

        QString getTable() const;
        void setDatabaseAndTable(const QString& database, const QString& table);

        Features features() const;
        QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateInsertQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateUpdateQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateDeleteQueryForItems(const QList<SqlQueryItem*>& items);
        bool supportsModifyingQueriesInMenu() const;

    protected:
        bool commitAddedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);
        bool commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);

        QString getDataSource();

    private:
        class CommitDeleteQueryBuilder : public CommitUpdateQueryBuilder
        {
            public:
                QString build();
        };

        class SelectColumnsQueryBuilder : public CommitUpdateQueryBuilder
        {
            public:
                QString build();
                void addColumn(const QString& col);
        };

        void updateColumnsAndValuesWithDefaultValues(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList,
                                                     QStringList& sqlValues, QList<QVariant>& args);
        void updateColumnsAndValues(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns,
                                    QStringList& colNameList, QStringList& sqlValues, QList<QVariant>& args);
        QString getInsertSql(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList, QStringList& sqlValues,
                             QList<QVariant>& args);
        void updateRowAfterInsert(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns, RowId rowId);
        bool processNullValueAfterInsert(SqlQueryItem* item, QVariant& value, const SqlQueryModelColumnPtr& modelColumn,
                                         QHash<SqlQueryModelColumnPtr,SqlQueryItem*>& columnsToReadFromDb, RowId rowId,
                                         Parser& parser);
        void processDefaultValueAfterInsert(QHash<SqlQueryModelColumnPtr,SqlQueryItem*>& columnsToReadFromDb, QHash<SqlQueryItem*,QVariant>& values,
                                            RowId rowId);

        QString table;
        bool isWithOutRowIdTable = false;
};

#endif // SQLTABLEMODEL_H
