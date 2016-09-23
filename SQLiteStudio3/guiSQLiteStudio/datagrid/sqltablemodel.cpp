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

    QString dbName = database;
    if (database.toLower() == "main" || database.isEmpty())
        dbName = QString();

    tablesInUse.clear();
    tablesInUse << DbAndTable(db, dbName, table);

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
    SqlQueryPtr result = db->exec(sql, args);

    // Handle error
    if (result->isError())
    {
        foreach (SqlQueryItem* item, itemsInRow)
            item->setCommitingError(true);

        notifyError(tr("Error while commiting new row: %1").arg(result->getErrorText()));
        return false;
    }

    // Reloading row with actual values (because of DEFAULT, AUTOINCR)
    RowId rowId;
    if (isWithOutRowIdTable)
    {
        SqlQueryItem* item = nullptr;
        int i = 0;
        for (const SqlQueryModelColumnPtr& modelColumn : modelColumns)
        {
            item = itemsInRow[i++];
            if (modelColumn->isPk())
                rowId[modelColumn->column] = item->getValue();
        }
    }
    else
        rowId = result->getInsertRowId();

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

    // This should not happen anymore (since WITHOUT ROWID tables should be handled properly now,
    // but we will keep this here for a while, just in case.
//    if (itemsInRow[0]->isJustInsertedWithOutRowId())
//    {
//        QString msg = tr("When inserted new row to the WITHOUT ROWID table, using DEFAULT value for PRIMARY KEY, "
//                         "the table has to be reloaded in order to delete the new row.");
//        notifyError(tr("Error while deleting row from table %1: %2").arg(table).arg(msg));
//        return false;
//    }

    RowId rowId = itemsInRow[0]->getRowId();
    if (rowId.isEmpty())
        return false;

    Dialect dialect = db->getDialect();

    CommitDeleteQueryBuilder queryBuilder;
    queryBuilder.setTable(wrapObjIfNeeded(table, dialect));
    queryBuilder.setRowId(rowId);

    QString sql = queryBuilder.build();
    QHash<QString, QVariant> args = queryBuilder.getQueryArgs();

    SqlQueryPtr result = db->exec(sql, args);
    if (result->isError())
    {
        notifyError(tr("Error while deleting row from table %1: %2").arg(table).arg(result->getErrorText()));
        return false;
    }

    if (!SqlQueryModel::commitDeletedRow(itemsInRow))
        qCritical() << "Could not delete row from SqlQueryView while commiting row deletion.";

    return true;
}

bool SqlTableModel::supportsModifyingQueriesInMenu() const
{
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
        conditions << wrapObjIfNeeded(column->column, dialect)+" LIKE '%"+escapeString(value)+"%'";

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
        conditions << wrapObjIfNeeded(column->column, dialect)+" REGEXP '"+escapeString(value)+"'";

    setQuery("SELECT * FROM "+getDataSource()+" WHERE "+conditions.join(" OR "));
    executeQuery();
}

void SqlTableModel::resetFilter()
{
    setQuery("SELECT * FROM "+getDataSource());
    //reload();
    executeQuery();
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

void SqlTableModel::updateRowAfterInsert(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns, RowId rowId)
{
    Dialect dialect = db->getDialect();

    // Update cells with data just like it was entered. Only DEFAULT and PRIMARY KEY AUTOINCREMENT will have special values.
    // If the DEFAULT is not an explicit literal, but an expression and db is SQLite3, we have to read the inserted value from DB.
    QHash<SqlQueryModelColumnPtr,SqlQueryItem*> columnsToReadFromDb;
    Parser parser(dialect);
    SqliteExpr* expr = nullptr;
    QHash<SqlQueryItem*,QVariant> values;
    SqlQueryItem* item = nullptr;
    int i = 0;
    for (const SqlQueryModelColumnPtr& modelColumn : modelColumns)
    {
        item = itemsInRow[i++];
//        qDebug() << "Item is for column" << item->getColumn()->column << ", column iterated:" << modelColumn->column;
        if (item->getValue().isNull())
        {
            if (modelColumn->isDefault())
            {
                if (dialect == Dialect::Sqlite3)
                {
                    expr = parser.parseExpr(modelColumn->getDefaultConstraint()->defaultValue);
                    if (expr && expr->mode != SqliteExpr::Mode::LITERAL_VALUE)
                    {
                        if (isWithOutRowIdTable && rowId.isEmpty())
                        {
                            qWarning() << "Inserted expression as DEFAULT value for table WITHOUT ROWID and actually no ROWID."
                                       << "This is currently unsupported to refresh such cell value instantly.";
                            values[item] = QVariant();
                        }
                        else
                            columnsToReadFromDb[modelColumn] = item;

                        continue;
                    }
                }
                values[item] = modelColumn->getDefaultConstraint()->defaultValue;
                continue;
            }

            // If this is the PK AUTOINCR column we use RowId as value, because it was skipped when setting values to items
            if (modelColumn->isPk() && modelColumn->isAutoIncr())
            {
                values[item] = rowId["ROWID"];
                continue;
            }
        }

        values[item] = item->getValue();
    }

    // Reading values for DEFAULT values being an expression
    if (columnsToReadFromDb.size() > 0)
    {
        // Preparing query
        static_qstring(limitedColTpl, "substr(%1, 1, %2)");
        SelectColumnsQueryBuilder queryBuilder;
        queryBuilder.setTable(wrapObjIfNeeded(table, dialect));
        queryBuilder.setRowId(rowId);
        QList<SqlQueryModelColumnPtr> columnKeys = columnsToReadFromDb.keys();
        for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
            queryBuilder.addColumn(limitedColTpl.arg(wrapObjIfNeeded(modelColumn->column, dialect), QString::number(cellDataLengthLimit)));

        // Executing query
        SqlQueryPtr defColValues = db->exec(queryBuilder.build(), queryBuilder.getQueryArgs(), Db::Flag::PRELOAD);

        // Handling error
        if (defColValues->isError())
        {
            qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. Error from database was:"
                        << defColValues->getErrorText();

            for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
                values[columnsToReadFromDb[modelColumn]] = QVariant();
        }
        else if (!defColValues->hasNext())
        {
            qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. There were no result rows.";

            for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
                values[columnsToReadFromDb[modelColumn]] = QVariant();
        }
        else
        {
            // Reading a row
            SqlResultsRowPtr row = defColValues->next();
            if (row->valueList().size() != columnKeys.size())
            {
                qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. Number of columns from results was invalid:"
                            << row->valueList().size() << ", while expected:" << columnKeys.size();

                for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
                    values[columnsToReadFromDb[modelColumn]] = QVariant();
            }
            else
            {
                int colIdx = 0;
                for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
                    values[columnsToReadFromDb[modelColumn]] = row->value(colIdx++);
            }
        }
    }

    // Update cell data with results
    int colIdx = 0;
    for (SqlQueryItem* itemToUpdate : itemsInRow)
    {
        updateItem(itemToUpdate, values[itemToUpdate], colIdx, rowId);

        if (isWithOutRowIdTable && rowId.isEmpty())
            itemToUpdate->setJustInsertedWithOutRowId(true);

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

    SqlQueryItem* item = nullptr;
    int i = 0;
    foreach (SqlQueryModelColumnPtr modelColumn, modelColumns)
    {
        item = itemsInRow[i++];
        if (item->getValue().isNull())
        {
            if (CFG_UI.General.UseDefaultValueForNull.get() && modelColumn->isDefault())
                continue;

            if (modelColumn->isNotNull()) // value is null, but it's NOT NULL, try using DEFAULT, or fail.
                continue;

            if (modelColumn->isPk() && modelColumn->isAutoIncr())
                continue;
        }

        colNameList << wrapObjIfNeeded(modelColumn->column, dialect);
        sqlValues << ":arg" + QString::number(i);
        args << item->getFullValue();
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
            SqlQueryPtr results = db->exec("SELECT max("+colName+") FROM "+tableName);
            qint64 rowid = 0;
            QVariant cellValue = results->getSingleCell();
            if (!cellValue.isNull())
                rowid = cellValue.toLongLong();

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
