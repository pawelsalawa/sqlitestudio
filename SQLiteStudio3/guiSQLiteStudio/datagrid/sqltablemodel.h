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
        void applyRegExpFilter(const QString& value);
        void resetFilter();
        QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);

    protected:
        bool commitAddedRow(const QList<SqlQueryItem*>& itemsInRow);
        bool commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow);

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
        QString getDatabasePrefix();
        QString getDataSource();

        QString table;
        QString database;
        bool isWithOutRowIdTable = false;
};

#endif // SQLTABLEMODEL_H
