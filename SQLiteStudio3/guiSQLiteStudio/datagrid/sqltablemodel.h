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

        void prepareColumnsAndBindParams(const QList<SqlQueryItem*>& itemsInRow,
                                    QStringList& colNameList, QStringList& sqlValues, QList<QVariant>& args);
        QString getInsertSql(QStringList& colNameList, QStringList& sqlValues);

        QString table;
        bool isWithOutRowIdTable = false;
};

#endif // SQLTABLEMODEL_H
