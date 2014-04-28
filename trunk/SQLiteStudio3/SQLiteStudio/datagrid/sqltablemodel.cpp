#include "sqltablemodel.h"
#include "common/utils_sql.h"
#include "sqlqueryitem.h"
#include "services/notifymanager.h"
#include <QDebug>
#include <QApplication>
#include <schemaresolver.h>

SqlTableModel::SqlTableModel(QObject *parent) :
    SqlQueryModel(parent)
{
}

QString SqlTableModel::getDatabase() const
{
    return database;
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

    SchemaResolver resolver(db);
    isWithOutRowIdTable = resolver.isWithoutRowIdTable(database, table);
}

SqlQueryModel::Features SqlTableModel::features() const
{
    return INSERT_ROW|DELETE_ROW|FILTERING;
}


bool SqlTableModel::commitAddedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    QList<SqlQueryModelColumnPtr> modelColumns = getTableColumnModels(table);
    if (modelColumns.size() != itemsInRow.size())
    {
        qCritical() << "Tried to SqlTableModel::commitAddedRow() with number of columns in argument different than model resolved for the table.";
        return false;
    }

    // Check that just in case:
    if (modelColumns.size() == 0)
    {
        qCritical() << "Tried to SqlTableModel::commitAddedRow() with number of resolved columns in the table equal to 0!";
        return false;
    }

    // Prepare column placeholders and their values
    QStringList colNameList;
    QStringList sqlValues;
    QList<QVariant> args;
    updateColumnsAndValues(itemsInRow, modelColumns, colNameList, sqlValues, args);

    // Prepare SQL query
    QString sql = getInsertSql(modelColumns, colNameList, sqlValues, args);

    // Execute query
    SqlResultsPtr result = db->exec(sql, args);

    // Handle error
    if (result->isError())
    {
        foreach (SqlQueryItem* item, itemsInRow)
            item->setCommitingError(true);

        notifyError(tr("Error while commiting new row: %1").arg(result->getErrorText()));
        return false;
    }

    // Reloading row with actual values (because of DEFAULT, AUTOINCR)
    RowId rowId = result->getInsertRowId();
    updateRowAfterInsert(itemsInRow, modelColumns, rowId);
    return true;
}

bool SqlTableModel::commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    if (itemsInRow.size() == 0)
    {
        qCritical() << "Tried to SqlTableModel::commitDeletedRow() with number of items equal to 0!";
        return false;
    }

    if (itemsInRow[0]->isJustInsertedWithOutRowId())
    {
        QString msg = tr("When inserted new row to the WITHOUT ROWID table, using DEFAULT value for PRIMARY KEY, "
                         "the table has to be reloaded in order to delete the new row.");
        notifyError(tr("Error while deleting row from table %1: %2").arg(table).arg(msg));
        return false;
    }

    RowId rowId = itemsInRow[0]->getRowId();
    if (rowId.isEmpty())
        return false;

    Dialect dialect = db->getDialect();

    CommitDeleteQueryBuilder queryBuilder;
    queryBuilder.setTable(wrapObjIfNeeded(table, dialect));
    queryBuilder.setRowId(rowId);

    QString sql = queryBuilder.build();
    QHash<QString, QVariant> args = queryBuilder.getQueryArgs();

    SqlResultsPtr result = db->exec(sql, args);
    if (result->isError())
    {
        notifyError(tr("Error while deleting row from table %1: %2").arg(table).arg(result->getErrorText()));
        return false;
    }

    if (!SqlQueryModel::commitDeletedRow(itemsInRow))
        qCritical() << "Could not delete row from SqlQueryView while commiting row deletion.";

    return true;
}

void SqlTableModel::applySqlFilter(const QString& value)
{
    if (value.isEmpty())
    {
        resetFilter();
        return;
    }

    setQuery("SELECT * FROM "+getDataSource()+" WHERE "+value);
    executeQuery();
}

void SqlTableModel::applyStringFilter(const QString& value)
{
    if (value.isEmpty())
    {
        resetFilter();
        return;
    }

    Dialect dialect = db->getDialect();
    QStringList conditions;
    foreach (SqlQueryModelColumnPtr column, columns)
        conditions << wrapObjIfNeeded(column->column, dialect)+" LIKE '%"+value+"%'";

    setQuery("SELECT * FROM "+getDataSource()+" WHERE "+conditions.join(" OR "));
    executeQuery();
}

void SqlTableModel::applyRegExpFilter(const QString& value)
{
    if (value.isEmpty())
    {
        resetFilter();
        return;
    }

    Dialect dialect = db->getDialect();
    QStringList conditions;
    foreach (SqlQueryModelColumnPtr column, columns)
        conditions << wrapObjIfNeeded(column->column, dialect)+" REGEXP '"+value+"'";

    setQuery("SELECT * FROM "+getDataSource()+" WHERE "+conditions.join(" OR "));
    executeQuery();
}

void SqlTableModel::resetFilter()
{
    setQuery("SELECT * FROM "+getDataSource());
    //reload();
    executeQuery();
}

void SqlTableModel::updateRowAfterInsert(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns, RowId rowId)
{
    // Update cells with data just like it was entered. Only DEFAULT and PRIMARY KEY AUTOINCREMENT will have special values.
    QList<QVariant> values;
    int i = 0;
    foreach (SqlQueryModelColumnPtr modelColumn, modelColumns)
    {
        if (itemsInRow[i]->getValue().isNull())
        {
            if (modelColumn->isDefault())
            {
                values << modelColumn->getDefaultConstraint()->defaultValue;
                continue;
            }

            // If this is the PK AUTOINCR column we use RowId as value, because it was skipped when setting values to items
            if (modelColumn->isPk() && modelColumn->isAutoIncr())
            {
                values << rowId;
                continue;
            }
        }

        values << itemsInRow[i]->getValue();
        i++;
    }

    // Update cell data with results
    int colIdx = 0;
    foreach (SqlQueryItem* item, itemsInRow)
    {
        updateItem(item, values[colIdx], colIdx, rowId);

        if (isWithOutRowIdTable && rowId.isEmpty())
            item->setJustInsertedWithOutRowId(true);

        colIdx++;
    }
}

QString SqlTableModel::getDatabasePrefix()
{
    if (database.isNull())
        return "main.";

    return wrapObjIfNeeded(database, db->getDialect()) + ".";
}

QString SqlTableModel::getDataSource()
{
    return getDatabasePrefix() + wrapObjIfNeeded(table, db->getDialect());
}

QString SqlTableModel::getInsertSql(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList,
                                    QStringList& sqlValues, QList<QVariant>& args)
{
    Dialect dialect = db->getDialect();

    QString sql = "INSERT INTO "+wrapObjIfNeeded(table, dialect);
    if (colNameList.size() == 0)
    {
        // There are all null values passed to the query. We need to use Sqlite3 special syntax, or find at least one default value
        if (dialect == Dialect::Sqlite2)
            updateColumnsAndValuesWithDefaultValues(modelColumns, colNameList, sqlValues, args);
        else // Sqlite3 has default values syntax for that case
            sql += " DEFAULT VALUES";
    }
    else
        sql += " ("+colNameList.join(", ")+") VALUES ("+sqlValues.join(", ")+")";

    return sql;
}

void SqlTableModel::updateColumnsAndValues(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns,
                                           QStringList& colNameList, QStringList& sqlValues, QList<QVariant>& args)
{
    Dialect dialect = db->getDialect();

    int i = 0;
    foreach (SqlQueryModelColumnPtr modelColumn, modelColumns)
    {
        if (itemsInRow[i]->getValue().isNull())
        {
            if (modelColumn->isDefault())
                continue;

            if (modelColumn->isPk() && modelColumn->isAutoIncr())
                continue;
        }

        colNameList << wrapObjIfNeeded(modelColumn->column, dialect);
        sqlValues << ":arg" + QString::number(i);
        args << itemsInRow[i++]->getFullValue();
    }
}

void SqlTableModel::updateColumnsAndValuesWithDefaultValues(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList,
                                                            QStringList& sqlValues, QList<QVariant>& args)
{
    Dialect dialect = db->getDialect();

    // First try to find the one with DEFAULT value
    foreach (SqlQueryModelColumnPtr modelColumn, modelColumns)
    {
        if (modelColumn->isDefault())
        {
            colNameList << wrapObjIfNeeded(modelColumn->column, dialect);
            sqlValues << ":defValue";
            args << modelColumn->getDefaultConstraint()->defaultValue;
            return;
        }
    }

    // No DEFAULT, try with AUTOINCR
    foreach (SqlQueryModelColumnPtr modelColumn, modelColumns)
    {
        if (modelColumn->isPk() && modelColumn->isAutoIncr())
        {
            QString colName = wrapObjIfNeeded(modelColumn->column, dialect);
            QString tableName = wrapObjIfNeeded(table, dialect);
            SqlResultsPtr results = db->exec("SELECT max("+colName+") FROM "+tableName);
            int rowid = 0;
            QVariant cellValue = results->getSingleCell();
            if (!cellValue.isNull())
                rowid = cellValue.toInt();

            colNameList << wrapObjIfNeeded(modelColumn->column, dialect);
            sqlValues << ":defValue";
            args << rowid;
            return;
        }
    }

    // No luck with AUTOINCR either, put NULL and if there's a NOT NULL in any column,
    // user will get the proper error message from Sqlite.
    colNameList << wrapObjIfNeeded(modelColumns[0]->column, dialect);
    sqlValues << ":defValue";
    args << QVariant();
}

QString SqlTableModel::CommitDeleteQueryBuilder::build()
{
    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database+".";

    dbAndTable += table;
    return "DELETE FROM  "+dbAndTable+" WHERE "+conditions.join(" AND ")+";";
}
