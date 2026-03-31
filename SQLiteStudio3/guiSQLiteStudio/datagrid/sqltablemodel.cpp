#include "sqltablemodel.h"
#include "common/utils_sql.h"
#include "sqlqueryitem.h"
#include "services/notifymanager.h"
#include "uiconfig.h"
#include <QDebug>
#include <QApplication>
#include <schemaresolver.h>
#include <querygenerator.h>

SqlTableModel::SqlTableModel(QObject *parent) :
    SqlDataSourceQueryModel(parent)
{
}

QString SqlTableModel::getTable() const
{
    return table;
}

void SqlTableModel::setDatabaseAndTable(const QString& database, const QString& table)
{
    this->database = database;
    this->table = table;
    setQuery("SELECT * FROM "+getDataSource());
    updateTablesInUse(table);

    SchemaResolver resolver(db);
    isWithOutRowIdTable = resolver.isWithoutRowIdTable(database, table);
}

SqlQueryModel::Features SqlTableModel::features() const
{
    return INSERT_ROW|DELETE_ROW|FILTERING;
}

bool SqlTableModel::commitAddedRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    if (columns.size() != itemsInRow.size())
    {
        qCritical() << "Tried to SqlTableModel::commitAddedRow() with number of columns in argument different than model resolved for the table.";
        return false;
    }

    // Check that just in case:
    if (columns.size() == 0)
    {
        qCritical() << "Tried to SqlTableModel::commitAddedRow() with number of resolved columns in the table equal to 0!";
        return false;
    }

    // Prepare column placeholders and their values
    QStringList colNameList;
    QStringList bindParams;
    QList<QVariant> args;
    prepareColumnsAndBindParams(itemsInRow, colNameList, bindParams, args);

    // Prepare SQL query
    QString sql = getInsertSql(colNameList, bindParams);

    // Execute query
    SqlQueryPtr result = db->exec(sql, args);

    // Handle error
    if (result->isError())
    {
        QString errMsg = tr("Error while committing new row: %1").arg(result->getErrorText());
        for (SqlQueryItem* item : itemsInRow)
            item->setCommittingError(true, errMsg);

        notifyError(errMsg);
        return false;
    }

    // Take values from RETURNING clause to get actual values (because of DEFAULT, AUTOINCR).
    QList<SqlResultsRowPtr> rows = result->getAll();
    if (rows.size() != 1)
    {
        qCritical() << "After inserting new row to the table, number of rows returned from the query != 1. This should not happen. Number:" << rows.size();
        return false;
    }

    QList<QVariant> rowValues = rows.first()->valueList();
    if (columns.size() != rowValues.size())
    {
        qCritical() << "Tried to SqlTableModel::commitAddedRow() with number of columns in argument different than RETURNING values from the INSERT.";
        return false;
    }

    // ROWID - either compound ID for WITHOUT ROWID table, or just ROWID for regular table
    RowId rowId;
    if (isWithOutRowIdTable)
    {
        int i = 0;
        for (SqlQueryModelColumnPtr& modelColumn : columns)
        {
            if (modelColumn->isPk())
                rowId[modelColumn->column] = rowValues[i];

            i++;
        }
    }
    else
        rowId = result->getInsertRowId();

    // After all items are committed successfully, update data/metadata for inserted rows/items
    successfulCommitHandlers << [this, itemsInRow, rowValues, rowId]()
    {
        int colIdx = 0;
        for (SqlQueryItem* itemToUpdate : itemsInRow)
        {
            updateItem(itemToUpdate, rowValues[colIdx], columns[colIdx], rowId);
            colIdx++;
        }
    };

    return true;
}

bool SqlTableModel::commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    if (itemsInRow.size() == 0)
    {
        qCritical() << "Tried to SqlTableModel::commitDeletedRow() with number of items equal to 0!";
        return false;
    }

    RowId rowId = itemsInRow[0]->getRowId();
    if (rowId.isEmpty())
        return false;

    CommitDeleteQueryBuilder queryBuilder;
    queryBuilder.setTable(wrapObjIfNeeded(table));
    queryBuilder.setRowId(rowId);

    QString sql = queryBuilder.build();
    QHash<QString, QVariant> args = queryBuilder.getQueryArgs();

    SqlQueryPtr result = db->exec(sql, args);
    if (result->isError())
    {
        QString errMsg = tr("Error while deleting row from table %1: %2").arg(table, result->getErrorText());
        for (SqlQueryItem* item : itemsInRow)
            item->setCommittingError(true, errMsg);

        notifyError(errMsg);
        return false;
    }

    if (!SqlQueryModel::commitDeletedRow(itemsInRow, successfulCommitHandlers))
        qCritical() << "Could not delete row from SqlQueryView while committing row deletion.";

    return true;
}

bool SqlTableModel::supportsModifyingQueriesInMenu() const
{
    return true;
}

QString SqlTableModel::generateSelectQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateSelectFromTable(db, database, table, values);
}

QString SqlTableModel::generateInsertQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateInsertToTable(db, database, table, values);
}

QString SqlTableModel::generateUpdateQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateUpdateOfTable(db, database, table, values);
}

QString SqlTableModel::generateDeleteQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateDeleteFromTable(db, database, table, values);
}

QString SqlTableModel::getDataSource()
{
    return getDatabasePrefix() + wrapObjIfNeeded(table);
}

QString SqlTableModel::getInsertSql(QStringList& colNameList, QStringList& sqlValues)
{
    static_qstring(insertTpl, "INSERT INTO %1 %2 %3 RETURNING *");
    static_qstring(valuesTpl, "VALUES (%1)");
    static_qstring(columnsTpl, "(%1)");

    QString wrappedTableName = wrapObjIfNeeded(table);
    QString colNames = colNameList.join(", ");
    if (colNameList.isEmpty())
        return insertTpl.arg(wrappedTableName, "", "DEFAULT VALUES");
    else
        return insertTpl.arg(wrappedTableName, columnsTpl.arg(colNames), valuesTpl.arg(sqlValues.join(", ")));
}

void SqlTableModel::prepareColumnsAndBindParams(const QList<SqlQueryItem*>& itemsInRow, QStringList& colNameList,
                                           QStringList& bindParams, QList<QVariant>& args)
{
    SqlQueryItem* item = nullptr;
    int i = 0;
    for (SqlQueryModelColumnPtr& modelColumn : columns)
    {
        item = itemsInRow[i++];
        if (!modelColumn->canEdit())
            continue;

        if (item->isUntouched() && modelColumn->hasDefaultValueForInsert())
            continue;

        colNameList << wrapObjIfNeeded(modelColumn->column);
        bindParams << ":arg" + QString::number(i);
        args << item->getValue();
    }
}

QString SqlTableModel::CommitDeleteQueryBuilder::build()
{
    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database+".";

    dbAndTable += table;
    QString conditions = RowIdConditionBuilder::build();

    static_qstring(sql, "DELETE FROM %1 WHERE %2;");
    return sql.arg(dbAndTable, conditions);
}


QString SqlTableModel::SelectColumnsQueryBuilder::build()
{
    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database+".";

    dbAndTable += table;
    QString conditions = RowIdConditionBuilder::build();

    static_qstring(sql, "SELECT %1 FROM %2 WHERE %3 LIMIT 1;");
    return sql.arg(columns.join(", "), dbAndTable, conditions);
}

void SqlTableModel::SelectColumnsQueryBuilder::addColumn(const QString& col)
{
    columns << col;
}
