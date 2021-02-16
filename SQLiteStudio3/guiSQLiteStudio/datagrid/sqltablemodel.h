#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include "guiSQLiteStudio_global.h"
#include "sqlquerymodel.h"

class GUI_API_EXPORT SqlTableModel : public SqlQueryModel
{
        Q_OBJECT
    public:
        explicit SqlTableModel(QObject *parent = 0);

        QString getDatabase() const;
        QString getTable() const;
        void setDatabaseAndTable(const QString& database, const QString& table);

        Features features() const;
        void applySqlFilter(const QString& value);
        void applyStringFilter(const QString& value);
        void applyStringFilter(const QStringList& values);
        void applyRegExpFilter(const QString& value);
        void applyRegExpFilter(const QStringList& values);
        void resetFilter();
        QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateInsertQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateUpdateQueryForItems(const QList<SqlQueryItem*>& items);
        QString generateDeleteQueryForItems(const QList<SqlQueryItem*>& items);
        bool supportsModifyingQueriesInMenu() const;

    protected:
        bool commitAddedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);
        bool commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);

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

        typedef std::function<QString(const QString&)> FilterValueProcessor;

        static QString stringFilterValueProcessor(const QString& value);
        static QString regExpFilterValueProcessor(const QString& value);

        void applyFilter(const QString& value, FilterValueProcessor valueProc);
        void applyFilter(const QStringList& values, FilterValueProcessor valueProc);
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

        QString getDatabasePrefix();
        QString getDataSource();

        QString table;
        QString database;
        bool isWithOutRowIdTable = false;
};

#endif // SQLTABLEMODEL_H
