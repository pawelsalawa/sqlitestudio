#include "sqltablemodel.h"
#include "common/utils_sql.h"
#include "sqlqueryitem.h"
#include "services/notifymanager.h"
#include "uiconfig.h"
#include "common/unused.h"
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
        QString errMsg = tr("Error while committing new row: %1").arg(result->getErrorText());
        for (SqlQueryItem* item : itemsInRow)
            item->setCommittingError(true, errMsg);

        notifyError(errMsg);
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

    // After all items are committed successfully, update data/metadata for inserted rows/items
    successfulCommitHandlers << [this, itemsInRow, modelColumns, rowId]()
    {
        updateRowAfterInsert(itemsInRow, modelColumns, rowId);
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

void SqlTableModel::updateRowAfterInsert(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns, RowId rowId)
{
    // Update cells with data just like it was entered. Only DEFAULT and PRIMARY KEY AUTOINCREMENT will have special values.
    // If the DEFAULT is not an explicit literal, but an expression and db is SQLite3, we have to read the inserted value from DB.
    QHash<SqlQueryModelColumnPtr,SqlQueryItem*> columnsToReadFromDb;
    Parser parser;
    QHash<SqlQueryItem*,QVariant> values;
    SqlQueryItem* item = nullptr;
    int i = 0;
    for (const SqlQueryModelColumnPtr& modelColumn : modelColumns)
    {
        item = itemsInRow[i++];
        if (processNullValueAfterInsert(item, values[item], modelColumn, columnsToReadFromDb, rowId, parser))
            continue;

        values[item] = item->getValue();
    }

    // Reading values for DEFAULT values being an expression
    if (columnsToReadFromDb.size() > 0)
        processDefaultValueAfterInsert(columnsToReadFromDb, values, rowId);

    // Reading values for GENERATED columns
    i = 0;
    QList<SqlQueryItem*> generatedColumnItems;
    for (const SqlQueryModelColumnPtr& modelColumn : modelColumns)
    {
        if (modelColumn->isGenerated())
            generatedColumnItems << itemsInRow[i++];
    }
    refreshGeneratedColumns(generatedColumnItems, values, rowId);

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

bool SqlTableModel::processNullValueAfterInsert(SqlQueryItem* item, QVariant& value, const SqlQueryModelColumnPtr& modelColumn,
                                                QHash<SqlQueryModelColumnPtr, SqlQueryItem*>& columnsToReadFromDb, RowId rowId, Parser& parser)
{
//    qDebug() << "Item is for column" << item->getColumn()->column << ", column iterated:" << modelColumn->column;
    if (!item->getValue().isNull())
        return false;

    // If this is the PK AUTOINCR column we use RowId as value, because it was skipped when setting values to items
    if (modelColumn->isPk() && modelColumn->isAutoIncr())
    {
        value = rowId["ROWID"];
        return true;
    }

    if (!CFG_UI.General.UseDefaultValueForNull.get() || !modelColumn->isDefault())
        return false;

    SqliteExpr* expr = parser.parseExpr(modelColumn->getDefaultConstraint()->defaultValue);
    if (expr && expr->mode != SqliteExpr::Mode::LITERAL_VALUE)
    {
        if (isWithOutRowIdTable && rowId.isEmpty())
        {
            qWarning() << "Inserted expression as DEFAULT value for table WITHOUT ROWID and actually no ROWID."
                       << "This is currently unsupported to refresh such cell value instantly.";
            value = QVariant();
        }
        else
            columnsToReadFromDb[modelColumn] = item;

        return true;
    }

    if (expr)
        value = expr->literalValue;
    else
        value = modelColumn->getDefaultConstraint()->defaultValue;

    if (value.userType() == QMetaType::QString)
        value = stripString(value.toString());

    return true;
}

void SqlTableModel::processDefaultValueAfterInsert(QHash<SqlQueryModelColumnPtr, SqlQueryItem*>& columnsToReadFromDb, QHash<SqlQueryItem*, QVariant>& values, RowId rowId)
{
    // Preparing query
    static_qstring(limitedColTpl, "substr(%1, 1, %2)");
    SelectColumnsQueryBuilder queryBuilder;
    queryBuilder.setTable(wrapObjIfNeeded(table));
    queryBuilder.setRowId(rowId);
    QList<SqlQueryModelColumnPtr> columnKeys = columnsToReadFromDb.keys();
    for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
        queryBuilder.addColumn(limitedColTpl.arg(wrapObjIfNeeded(modelColumn->column), QString::number(cellDataLengthLimit)));

    // Executing query
    SqlQueryPtr defColValues = db->exec(queryBuilder.build(), queryBuilder.getQueryArgs(), Db::Flag::PRELOAD);

    // Handling error
    if (defColValues->isError())
    {
        qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. Error from database was:"
                    << defColValues->getErrorText();

        for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
            values[columnsToReadFromDb[modelColumn]] = QVariant();

        return;
    }


    if (!defColValues->hasNext())
    {
        qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. There were no result rows.";

        for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
            values[columnsToReadFromDb[modelColumn]] = QVariant();

        return;
    }


    // Reading a row
    SqlResultsRowPtr row = defColValues->next();
    if (row->valueList().size() != columnKeys.size())
    {
        qCritical() << "Could not load inserted values for DEFAULT expression in the table, so filling them with NULL. Number of columns from results was invalid:"
                    << row->valueList().size() << ", while expected:" << columnKeys.size();

        for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
            values[columnsToReadFromDb[modelColumn]] = QVariant();

        return;
    }

    int colIdx = 0;
    for (const SqlQueryModelColumnPtr& modelColumn : columnKeys)
        values[columnsToReadFromDb[modelColumn]] = row->value(colIdx++);
}

QString SqlTableModel::getDataSource()
{
    return getDatabasePrefix() + wrapObjIfNeeded(table);
}

QString SqlTableModel::getInsertSql(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList,
                                    QStringList& sqlValues, QList<QVariant>& args)
{
    UNUSED(modelColumns);
    UNUSED(args);
    QString sql = "INSERT INTO "+wrapObjIfNeeded(table);
    if (colNameList.size() == 0)
    {
        // There are all null values passed to the query. We need to use Sqlite3 special syntax, or find at least one default value
        sql += " DEFAULT VALUES";
    }
    else
        sql += " ("+colNameList.join(", ")+") VALUES ("+sqlValues.join(", ")+")";

    return sql;
}

void SqlTableModel::updateColumnsAndValues(const QList<SqlQueryItem*>& itemsInRow, const QList<SqlQueryModelColumnPtr>& modelColumns,
                                           QStringList& colNameList, QStringList& sqlValues, QList<QVariant>& args)
{
    SqlQueryItem* item = nullptr;
    int i = 0;
    for (SqlQueryModelColumnPtr modelColumn : modelColumns)
    {
        item = itemsInRow[i++];
        if (!modelColumn->canEdit())
            continue;

        if (item->getValue().isNull())
        {
            if (CFG_UI.General.UseDefaultValueForNull.get() && modelColumn->isDefault())
                continue;

            if (modelColumn->isNotNull()) // value is null, but it's NOT NULL, try using DEFAULT, or fail.
                continue;

            if (modelColumn->isPk() && modelColumn->isAutoIncr())
                continue;
        }

        colNameList << wrapObjIfNeeded(modelColumn->column);
        sqlValues << ":arg" + QString::number(i);
        args << item->getValue();
    }
}

void SqlTableModel::updateColumnsAndValuesWithDefaultValues(const QList<SqlQueryModelColumnPtr>& modelColumns, QStringList& colNameList,
                                                            QStringList& sqlValues, QList<QVariant>& args)
{
    // First try to find the one with DEFAULT value
    for (SqlQueryModelColumnPtr modelColumn : modelColumns)
    {
        if (modelColumn->isDefault())
        {
            colNameList << wrapObjIfNeeded(modelColumn->column);
            sqlValues << ":defValue";
            args << modelColumn->getDefaultConstraint()->defaultValue;
            return;
        }
    }

    // No DEFAULT, try with AUTOINCR
    for (SqlQueryModelColumnPtr modelColumn : modelColumns)
    {
        if (modelColumn->isPk() && modelColumn->isAutoIncr())
        {
            QString colName = wrapObjIfNeeded(modelColumn->column);
            QString tableName = wrapObjIfNeeded(table);
            SqlQueryPtr results = db->exec("SELECT max("+colName+") FROM "+tableName);
            qint64 rowid = 0;
            QVariant cellValue = results->getSingleCell();
            if (!cellValue.isNull())
                rowid = cellValue.toLongLong();

            colNameList << wrapObjIfNeeded(modelColumn->column);
            sqlValues << ":defValue";
            args << rowid;
            return;
        }
    }

    // No luck with AUTOINCR either, put NULL and if there's a NOT NULL in any column,
    // user will get the proper error message from Sqlite.
    colNameList << wrapObjIfNeeded(modelColumns[0]->column);
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
