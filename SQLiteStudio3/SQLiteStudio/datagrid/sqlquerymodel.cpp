#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "schemaresolver.h"
#include "common/unused.h"
#include "db/sqlerrorcodes.h"
#include "parser/ast/sqlitecreatetable.h"
#include "uiconfig.h"
#include "datagrid/sqlqueryview.h"
#include "datagrid/sqlqueryrownummodel.h"
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QMutableListIterator>
#include <QInputDialog>
#include <QTime>

SqlQueryModel::SqlQueryModel(QObject *parent) :
    QStandardItemModel(parent)
{
    queryExecutor = new QueryExecutor();
    queryExecutor->setDataLengthLimit(cellDataLengthLimit);
    connect(queryExecutor, &QueryExecutor::executionFinished, this, &SqlQueryModel::handleExecFinished);
    connect(queryExecutor, &QueryExecutor::executionFailed, this, &SqlQueryModel::handleExecFailed);
    connect(queryExecutor, &QueryExecutor::resultsCountingFinished, this, &SqlQueryModel::resultsCountingFinished);
    setItemPrototype(new SqlQueryItem());
}

SqlQueryModel::~SqlQueryModel()
{
    delete queryExecutor;
    queryExecutor = nullptr;
}

void SqlQueryModel::staticInit()
{
}

QString SqlQueryModel::getQuery() const
{
    return query;
}

void SqlQueryModel::setQuery(const QString &value)
{
    query = value;
}

void SqlQueryModel::setExplainMode(bool explain)
{
    this->explain = explain;
}

void SqlQueryModel::executeQuery()
{
    if (queryExecutor->isExecutionInProgress())
    {
        notifyWarn(tr("Only one query can be executed simultaneously."));
        return;
    }

    sortOrder.clear();
    queryExecutor->setSkipRowCounting(false);
    queryExecutor->setSortOrder(sortOrder);
    queryExecutor->setPage(0);
    reloading = false;

    executeQueryInternal();
}

void SqlQueryModel::executeQueryInternal()
{
    if (!db || !db->isValid())
    {
        notifyWarn("Cannot execute query on undefined or invalid database.");
        return;
    }

    if (query.isEmpty())
    {
        notifyWarn("Cannot execute empty query.");
        return;
    }

    emit executionStarted();

    queryExecutor->setQuery(query);
    queryExecutor->setResultsPerPage(CFG_UI.General.NumberOfRowsPerPage.get());
    queryExecutor->setExplainMode(explain);
    queryExecutor->setPreloadResults(true);
    queryExecutor->exec();
}

void SqlQueryModel::interrupt()
{
    queryExecutor->interrupt();
}

qint64 SqlQueryModel::getExecutionTime()
{
    return lastExecutionTime;
}

qint64 SqlQueryModel::getTotalRowsReturned()
{
    return totalRowsReturned;
}

qint64 SqlQueryModel::getTotalRowsAffected()
{
    return rowsAffected;
}

qint64 SqlQueryModel::getTotalPages()
{
    return totalPages;
}

QList<SqlQueryModelColumnPtr> SqlQueryModel::getColumns()
{
    return columns;
}

SqlQueryItem *SqlQueryModel::itemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<SqlQueryItem*>(QStandardItemModel::itemFromIndex(index));
}

SqlQueryItem*SqlQueryModel::itemFromIndex(int row, int column) const
{
    return dynamic_cast<SqlQueryItem*>(item(row, column));
}

int SqlQueryModel::getCellDataLengthLimit()
{
    return cellDataLengthLimit;
}

QModelIndexList SqlQueryModel::findIndexes(int role, const QVariant& value, int hits) const
{
    QModelIndex startIdx = index(0, 0);
    QModelIndex endIdx = index(rowCount() - 1, columnCount() - 1);
    return findIndexes(startIdx, endIdx, role, value, hits);
}

QModelIndexList SqlQueryModel::findIndexes(const QModelIndex& start, const QModelIndex& end, int role, const QVariant& value, int hits) const
{
    QModelIndexList results;
    bool allHits = hits < 0;
    QModelIndex parentIdx = parent(start);
    int fromRow = start.row();
    int toRow = end.row();
    int fromCol = start.column();
    int toCol = end.column();

    for (int row = fromRow; row <= toRow && (allHits || results.count() < hits); row++)
    {
        for (int col = fromCol; col <= toCol && (allHits || results.count() < hits); col++)
        {
            QModelIndex idx = index(row, col, parentIdx);
            if (!idx.isValid())
                 continue;

            QVariant cellVal = data(idx, role);
            if (value != cellVal)
                continue;

            results.append(idx);
        }
    }

    return results;
}

QList<SqlQueryItem*> SqlQueryModel::findItems(int role, const QVariant& value, int hits) const
{
    return toItemList(findIndexes(role, value, hits));
}

QList<SqlQueryItem*> SqlQueryModel::findItems(const QModelIndex& start, const QModelIndex& end, int role, const QVariant& value, int hits) const
{
    return toItemList(findIndexes(start, end, role, value, hits));
}

QList<SqlQueryItem*> SqlQueryModel::getUncommitedItems() const
{
    return findItems(SqlQueryItem::DataRole::UNCOMMITED, true);
}

QList<QList<SqlQueryItem*> > SqlQueryModel::groupItemsByRows(const QList<SqlQueryItem*>& items)
{
    QHash<int,QList<SqlQueryItem*> > itemsByRow;
    foreach (SqlQueryItem* item, items)
        itemsByRow[item->row()] << item;

    return itemsByRow.values();
}

QList<SqlQueryItem*> SqlQueryModel::filterOutCommitedItems(const QList<SqlQueryItem*>& items)
{
    // This method doesn't make use of QMutableListIterator to remove items from passed list,
    // because it would require list in argument to drop 'const' keyword and it's already
    // there in calling methods, so it's easier to copy list and filter on the fly.
    QList<SqlQueryItem*> newList;
    foreach (SqlQueryItem* item, items)
        if (item->isUncommited())
            newList << item;

    return newList;
}

QList<SqlQueryItem*> SqlQueryModel::getRow(int row)
{
    QList<SqlQueryItem*> items;
    for (int i = 0; i < columnCount(); i++)
        items << itemFromIndex(row, i);

    return items;
}

SqlQueryModel::Features SqlQueryModel::features() const
{
    return Features();
}

QList<SqlQueryItem*> SqlQueryModel::toItemList(const QModelIndexList& indexes) const
{
    QList<SqlQueryItem*> list;
    foreach (const QModelIndex& idx, indexes)
        list << itemFromIndex(idx);

    return list;
}

void SqlQueryModel::commit()
{
    QList<SqlQueryItem*> items = findItems(SqlQueryItem::DataRole::UNCOMMITED, true);
    commitInternal(items);
}

void SqlQueryModel::commit(const QList<SqlQueryItem*>& items)
{
    commitInternal(filterOutCommitedItems(items));
}

bool SqlQueryModel::commitRow(const QList<SqlQueryItem*>& itemsInRow)
{
    const SqlQueryItem* item = itemsInRow.at(0);
    if (!item)
    {
        qWarning() << "null item while call to commitRow() method. It shouldn't happen.";
        return true;
    }
    if (item->isNewRow())
        return commitAddedRow(getRow(item->row())); // we need to get all items again, in case of selective commit
    else if (item->isDeletedRow())
        return commitDeletedRow(getRow(item->row())); // we need to get all items again, in case of selective commit
    else
        return commitEditedRow(itemsInRow);
}

void SqlQueryModel::rollbackRow(const QList<SqlQueryItem*>& itemsInRow)
{
    const SqlQueryItem* item = itemsInRow.at(0);
    if (!item)
    {
        qWarning() << "null item while call to rollbackRow() method. It shouldn't happen.";
        return;
    }
    if (item->isNewRow())
        rollbackAddedRow(getRow(item->row())); // we need to get all items again, in case of selective commit
    else if (item->isDeletedRow())
        rollbackDeletedRow(getRow(item->row())); // we need to get all items again, in case of selective commit
    else
        rollbackEditedRow(itemsInRow);
}

void SqlQueryModel::rollback()
{
    QList<SqlQueryItem*> items = findItems(SqlQueryItem::DataRole::UNCOMMITED, true);
    rollbackInternal(items);
}

void SqlQueryModel::rollback(const QList<SqlQueryItem*>& items)
{
    rollbackInternal(filterOutCommitedItems(items));
}

void SqlQueryModel::commitInternal(const QList<SqlQueryItem*>& items)
{
    Db* db = getDb();
    if (!db->isOpen())
    {
        notifyError(tr("Cannot commit the data for a cell that refers to the already closed database."));
        return;
    }

    if (!db->begin())
    {
        notifyError(tr("Could not begin transaction on the database. Details: %1").arg(db->getErrorText()));
        return;
    }

    QList<QList<SqlQueryItem*> > groupedItems = groupItemsByRows(items);
    bool ok = true;
    foreach (const QList<SqlQueryItem*>& itemsInRow, groupedItems)
    {
        if (!commitRow(itemsInRow))
        {
            ok = false;
            break;
        }
    }

    // Getting current uncommited list (after rows deletion it may be different)
    QList<SqlQueryItem*> itemsLeft = findItems(SqlQueryItem::DataRole::UNCOMMITED, true);

    // Getting common elements of initial and current item list, because of a possibility of the selective commit
    QMutableListIterator<SqlQueryItem*> it(itemsLeft);
    while (it.hasNext())
    {
        if (!items.contains(it.next()))
            it.remove();
    }

    // Commiting to th database
    if (ok)
    {
        if (!db->commit())
        {
            ok = false;
            notifyError(tr("An error occurred while commiting the transaction: %1").arg(db->getErrorText()));
        }
        else
        {
            // Commited successfly
            foreach (SqlQueryItem* item, itemsLeft)
            {
                item->setUncommited(false);
                item->setNewRow(false);
            }

            emit commitStatusChanged(getUncommitedItems().size() > 0);
        }
    }

    if (!ok)
    {
        if (!db->rollback())
        {
            notifyError(tr("An error occurred while rolling back the transaction: %1").arg(db->getErrorText()));
            // Nothing else we can do about it, but it should not happen.
        }
    }
}

void SqlQueryModel::rollbackInternal(const QList<SqlQueryItem*>& items)
{
    QList<QList<SqlQueryItem*> > groupedItems = groupItemsByRows(items);
    foreach (const QList<SqlQueryItem*>& itemsInRow, groupedItems)
        rollbackRow(itemsInRow);

    emit commitStatusChanged(getUncommitedItems().size() > 0);
}

void SqlQueryModel::reload()
{
    queryExecutor->setSkipRowCounting(false);
    reloadInternal();
}

void SqlQueryModel::reloadInternal()
{
    if (!reloadAvailable)
        return;

    if (queryExecutor->isExecutionInProgress())
    {
        notifyWarn(tr("Only one query can be executed simultaneously."));
        return;
    }
    reloading = true;
    executeQueryInternal();
}

SqlQueryView* SqlQueryModel::getView() const
{
    return view;
}

void SqlQueryModel::setView(SqlQueryView* value)
{
    view = value;
    view->setModel(this);
}

int SqlQueryModel::getCurrentPage(bool includeOneBeingLoaded) const
{
    int result = includeOneBeingLoaded ? queryExecutor->getPage() : page;
    return result < 0 ? 0 : result;
}

bool SqlQueryModel::commitAddedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    UNUSED(itemsInRow);
    return false;
}

bool SqlQueryModel::commitEditedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    if (itemsInRow.size() == 0)
    {
        qWarning() << "SqlQueryModel::commitEditedRow() called with no items in the list.";
        return true;
    }

    Dialect dialect = db->getDialect();

    // Values
    QString query;
    SqlQueryModelColumn* col;
    QHash<QString,QVariant> queryArgs;
    RowId rowId;
    CommitUpdateQueryBuilder queryBuilder;
    foreach (SqlQueryItem* item, itemsInRow)
    {
        col = item->getColumn();
        if (col->editionForbiddenReason.size() > 0 || item->isJustInsertedWithOutRowId())
        {
            notifyError(tr("Tried to commit a cell which is not editable (yet modified and waiting for commit)! This is a bug. Please report it."));
            return false;
        }

        // RowId
        queryBuilder.clear();
        rowId = item->getRowId();
        queryBuilder.setRowId(rowId);

        // Database and table
        queryBuilder.setTable(wrapObjIfNeeded(col->table, dialect));
        if (!col->database.isNull())
            queryBuilder.setDatabase(wrapObjIfNeeded(col->database, dialect));

        // Completing query
        queryBuilder.setColumn(wrapObjIfNeeded(col->column, dialect));
        query = queryBuilder.build();

        // Adding updated value to arguments
        queryArgs = queryBuilder.getQueryArgs();
        queryArgs[":value"] = item->getValue();

        // Get the data
        SqlQueryPtr results = db->exec(query, queryArgs);
        if (results->isError())
        {
            item->setCommitingError(true);
            notifyError(tr("An error occurred while commiting the data: %1").arg(results->getErrorText()));
            return false;
        }
    }

    return true;
}

bool SqlQueryModel::commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    if (itemsInRow.size() == 0)
    {
        qCritical() << "No items passed to SqlQueryModel::commitDeletedRow().";
        return false;
    }

    int row = itemsInRow[0]->index().row();
    return removeRow(row);
}

void SqlQueryModel::rollbackAddedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    if (itemsInRow.size() == 0)
    {
        qCritical() << "No items passed to SqlQueryModel::rollbackAddedRow().";
        return;
    }

    int row = itemsInRow[0]->index().row();
    removeRow(row);
}

void SqlQueryModel::rollbackEditedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    foreach (SqlQueryItem* item, itemsInRow)
        item->rollback();
}

void SqlQueryModel::rollbackDeletedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    foreach (SqlQueryItem* item, itemsInRow)
        item->rollback();
}

SqlQueryModelColumnPtr SqlQueryModel::getColumnModel(const QString& database, const QString& table, const QString& column)
{
    Column colObj(database, table, column);
    if (columnMap.contains(colObj))
        return columnMap.value(colObj);

    return SqlQueryModelColumnPtr();
}

SqlQueryModelColumnPtr SqlQueryModel::getColumnModel(const QString& table, const QString& column)
{
    return getColumnModel("main", table, column);
}

QList<SqlQueryModelColumnPtr> SqlQueryModel::getTableColumnModels(const QString& database, const QString& table)
{
    QList<SqlQueryModelColumnPtr> results;
    foreach (SqlQueryModelColumnPtr modelColumn, columns)
    {
        if (modelColumn->database.compare(database, Qt::CaseInsensitive) != 0)
            continue;

        if (modelColumn->table.compare(table, Qt::CaseInsensitive) != 0)
            continue;

        results << modelColumn;
    }
    return results;
}

QList<SqlQueryModelColumnPtr> SqlQueryModel::getTableColumnModels(const QString& table)
{
    return getTableColumnModels("main", table);
}

void SqlQueryModel::loadData(SqlQueryPtr results)
{
    if (rowCount() > 0)
        clear();

    view->horizontalHeader()->show();

    // Read columns first. It will be needed later.
    readColumns();

    // Load data
    SqlResultsRowPtr row;
    int rowIdx = 0;
    int rowsPerPage = CFG_UI.General.NumberOfRowsPerPage.get();
    rowNumBase = getCurrentPage() * rowsPerPage + 1;

    updateColumnHeaderLabels();
    QList<QStandardItem*> itemList;
    while (results->hasNext() && rowIdx < rowsPerPage)
    {
        row = results->next();
        itemList = loadRow(row);
        insertRow(rowIdx, itemList);

        if ((rowIdx % 100) == 0)
            qApp->processEvents();

        rowIdx++;
    }

//    updateColumnsHeader();
}

QList<QStandardItem*> SqlQueryModel::loadRow(SqlResultsRowPtr row)
{
    QList<QStandardItem*> itemList;
    SqlQueryItem* item;
    RowId rowId;
    int colIdx = 0;
    foreach (const QVariant& value, row->valueList().mid(rowIdColumns))
    {
        item = new SqlQueryItem();
        rowId = getRowIdValue(row, colIdx);
        updateItem(item, value, colIdx, rowId);
        itemList << item;
        colIdx++;
    }

    return itemList;
}

RowId SqlQueryModel::getRowIdValue(SqlResultsRowPtr row, int columnIdx)
{
    RowId rowId;
    Table table = tablesForColumns[columnIdx];
    QHash<QString,QString> rowIdColumns = tableToRowIdColumn[table];
    QHashIterator<QString,QString> it(rowIdColumns);
    QString col;
    while (it.hasNext())
    {
        // Check if the result row contains QueryExecutor's column alias for this RowId column
        col = it.next().key();
        if (row->contains(col))
        {
            // It does, do let's put the actual column name into the RowId and assign the RowId value to it.
            // Using the actucal column name as a key will let create a proper query for updates, etc, later on.
            rowId[it.value()] = row->value(col);
        }
        else if (columnEditionStatus[columnIdx])
        {
            qCritical() << "No row ID column for cell that is editable. Asked for row ID column named:" << col
                        << "in table" << tablesForColumns[columnIdx].getTable();
            return RowId();
        }
    }
    return rowId;
}

void SqlQueryModel::updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId)
{
    SqlQueryModelColumnPtr column = columns[columnIndex];
    Qt::Alignment alignment;

    if (column->isNumeric() && isNumeric(value))
        alignment = Qt::AlignRight|Qt::AlignVCenter;
    else
        alignment = Qt::AlignLeft|Qt::AlignVCenter;

    // This should be equal at most, unless we have UTF-8 string, than there might be more bytes.
    // If less, than it's not limited.
    bool limited = value.toByteArray().size() >= cellDataLengthLimit;

    item->setJustInsertedWithOutRowId(false);
    item->setValue(value, limited, true);
    item->setColumn(column.data());
    item->setTextAlignment(alignment);
    item->setRowId(rowId);
}

void SqlQueryModel::readColumns()
{
    columns.clear();
    tableToRowIdColumn.clear();

    // Reading column mapping for ROWID columns
    int totalRowIdCols = 0;
    Table table;
    foreach (const QueryExecutor::ResultRowIdColumnPtr& resCol, queryExecutor->getRowIdResultColumns())
    {
        table.setDatabase(resCol->database);
        table.setTable(resCol->table);
        tableToRowIdColumn[table] = resCol->queryExecutorAliasToColumn;
        totalRowIdCols += resCol->queryExecutorAliasToColumn.size();
    }

    // Reading column details (datatype, constraints)
    readColumnDetails();

    // Preparing other usful information about columns
    rowIdColumns = totalRowIdCols;
    tablesForColumns = getTablesForColumns();
    columnEditionStatus = getColumnEditionEnabledList();
}

void SqlQueryModel::readColumnDetails()
{
    // Preparing global (table oriented) edition forbidden reasons
    QSet<SqlQueryModelColumn::EditionForbiddenReason> editionForbiddenGlobalReasons;
    foreach (QueryExecutor::EditionForbiddenReason reason, queryExecutor->getEditionForbiddenGlobalReasons())
        editionForbiddenGlobalReasons << SqlQueryModelColumn::convert(reason);

    // Reading all the details from query executor source tables
    QHash<Table, TableDetails> tableDetails = readTableDetails();

    // Preparing for processing
    Table table;
    Column column;
    TableDetails details;
    TableDetails::ColumnDetails colDetails;

    SqlQueryModelColumnPtr modelColumn;
    SqliteColumnTypePtr modelColumnType;
    SqlQueryModelColumn::Constraint* modelConstraint;

    foreach (const QueryExecutor::ResultColumnPtr& resCol, queryExecutor->getResultColumns())
    {
        // Creating new column for the model (this includes column oriented forbidden reasons)
        modelColumn = SqlQueryModelColumnPtr::create(resCol);

        // Adding global edition forbidden reasons
        modelColumn->editionForbiddenReason += editionForbiddenGlobalReasons;

        // Getting details of given table and column
        table = Table(modelColumn->database, modelColumn->table);
        column = Column(modelColumn->database, modelColumn->table, modelColumn->column);

        details = tableDetails[table];
        colDetails = details.columns[modelColumn->column];

        // Column type
        modelColumnType = colDetails.type;
        if (modelColumnType)
            modelColumn->dataType = DataType(modelColumnType->name, modelColumnType->precision, modelColumnType->scale);

        // Column constraints
        foreach (SqliteCreateTable::Column::ConstraintPtr constrPtr, colDetails.constraints)
        {
            modelConstraint = SqlQueryModelColumn::Constraint::create(constrPtr);
            if (modelConstraint)
                modelColumn->constraints << modelConstraint;
        }

        // Table constraints
        foreach (SqliteCreateTable::ConstraintPtr constrPtr, details.constraints)
        {
            modelConstraint = SqlQueryModelColumn::Constraint::create(modelColumn->column, constrPtr);
            if (modelConstraint)
                modelColumn->constraints << modelConstraint;
        }

        // Adding to list for ordered access
        columns << modelColumn;

        // Adding to hash for fast, key based access
        columnMap[column] = modelColumn;
    }
}

QHash<Table, SqlQueryModel::TableDetails> SqlQueryModel::readTableDetails()
{
    QHash<Table, TableDetails> results;
    SqliteQueryPtr query;
    SqliteCreateTablePtr createTable;
    Dialect dialect = db->getDialect();
    SchemaResolver resolver(getDb());
    QString database;
    Table table;
    QString columnName;

    foreach (const QueryExecutor::SourceTablePtr& srcTable, queryExecutor->getSourceTables())
    {
        database = srcTable->database.isEmpty() ? "main" : srcTable->database;

        query = resolver.getParsedObject(database, srcTable->table);
        if (!query || !query.dynamicCast<SqliteCreateTable>())
        {
            qWarning() << "Could not get parsed table while reading table details in SqlQueryModel. Queried table was:"
                       << database + "." + srcTable->table;
            continue;
        }
        createTable = query.dynamicCast<SqliteCreateTable>();

        // Table details
        TableDetails tableDetails;
        table = {database, srcTable->table};

        // Table constraints
        foreach (SqliteCreateTable::Constraint* tableConstr, createTable->constraints)
            tableDetails.constraints << tableConstr->detach<SqliteCreateTable::Constraint>();

        // Table columns
        foreach (SqliteCreateTable::Column* columnStmt, createTable->columns)
        {
            // Column details
            TableDetails::ColumnDetails columnDetails;
            columnName = stripObjName(columnStmt->name, dialect);

            // Column type
            if (columnStmt->type)
                columnDetails.type = columnStmt->type->detach<SqliteColumnType>();
            else
                columnDetails.type = SqliteColumnTypePtr();

            // Column constraints
            foreach (SqliteCreateTable::Column::Constraint* columnConstr, columnStmt->constraints)
                columnDetails.constraints << columnConstr->detach<SqliteCreateTable::Column::Constraint>();

            tableDetails.columns[columnName] = columnDetails;
        }

        results[table] = tableDetails;
    }

    return results;

}

QList<Table> SqlQueryModel::getTablesForColumns()
{
    QList<Table> columnTables;
    Table table;
    foreach (SqlQueryModelColumnPtr column, columns)
    {
        if (column->editionForbiddenReason.size() > 0)
        {
            columnTables << Table();
            continue;
        }
        table = Table(column->database, column->table);
        columnTables << table;
    }
    return columnTables;
}

QList<bool> SqlQueryModel::getColumnEditionEnabledList()
{
    QList<bool> columnEditionEnabled;
    foreach (SqlQueryModelColumnPtr column, columns)
        columnEditionEnabled << (column->editionForbiddenReason.size() == 0);

    return columnEditionEnabled;
}

void SqlQueryModel::updateColumnsHeader()
{
    QueryExecutor::SortList executorSortOrder = queryExecutor->getSortOrder();
    if (executorSortOrder.size() > 0)
        emit sortingUpdated(executorSortOrder);
}

void SqlQueryModel::updateColumnHeaderLabels()
{
    headerColumns.clear();
    foreach (SqlQueryModelColumnPtr column, columns)
    {
        headerColumns << column->displayName;
    }

    setColumnCount(headerColumns.size());
}

void SqlQueryModel::handleExecFinished(SqlQueryPtr results)
{
    if (results->isError())
    {
        emit executionFailed(tr("Error while executing SQL query: %1").arg(results->getErrorText()));
        return;
    }

    storeStep1NumbersFromExecution();
    loadData(results);
    storeStep2NumbersFromExecution();

    reloadAvailable = true;

    emit loadingEnded(true);
    restoreNumbersToQueryExecutor();
    if (!reloading)
        emit executionSuccessful();

    reloading = false;

    if (queryExecutor->isRowCountingRequired() || rowCount() < CFG_UI.General.NumberOfRowsPerPage.get())
        emit totalRowsAndPagesAvailable(); // rows were counted manually
    else
        queryExecutor->countResults();
}

void SqlQueryModel::handleExecFailed(int code, QString errorMessage)
{
    UNUSED(code);

    if (rowCount() > 0)
    {
        clear();
        columns.clear();
        updateColumnHeaderLabels();
        view->horizontalHeader()->hide();
    }

    emit loadingEnded(false);
    if (reloading)
    {
        // If we were reloading, but it was interrupted, we don't want message about it.
        if (!SqlErrorCode::isInterrupted(code))
            emit executionFailed(tr("Error while loading query results: %1").arg(errorMessage));
    }
    else
        emit executionFailed(tr("Error while executing SQL query: %1").arg(errorMessage));

    restoreNumbersToQueryExecutor();
    resultsCountingFinished(0, 0, 0);

    reloading = false;
}

void SqlQueryModel::resultsCountingFinished(quint64 rowsAffected, quint64 rowsReturned, int totalPages)
{
    this->rowsAffected = rowsAffected;
    this->totalRowsReturned = rowsReturned;
    this->totalPages = totalPages;
    emit totalRowsAndPagesAvailable();
}

void SqlQueryModel::itemValueEdited(SqlQueryItem* item)
{
    UNUSED(item);
    emit commitStatusChanged(getUncommitedItems().size() > 0);
}

void SqlQueryModel::changeSorting(int logicalIndex, Qt::SortOrder order)
{
    if (!reloadAvailable)
        return;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setSortOrder({QueryExecutor::Sort(order, logicalIndex)});
    reloadInternal();
}

void SqlQueryModel::changeSorting(int logicalIndex)
{
    Qt::SortOrder newOrder = Qt::AscendingOrder;
    if (sortOrder.size() == 1)
    {
        switch (sortOrder.first().order)
        {
            case QueryExecutor::Sort::ASC:
                newOrder = Qt::DescendingOrder;
                break;
            case QueryExecutor::Sort::DESC:
                newOrder = Qt::AscendingOrder;
                break;
            case QueryExecutor::Sort::NONE:
                newOrder = Qt::AscendingOrder;
                break;
        }
    }
    changeSorting(logicalIndex, newOrder);
}

void SqlQueryModel::firstPage()
{
    if (!reloadAvailable)
        return;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setPage(0);
    reloadInternal();
}

void SqlQueryModel::prevPage()
{
    if (!reloadAvailable)
        return;

    int newPage = page - 1;
    if (newPage < 0)
        newPage = 0;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setPage(newPage);
    reloadInternal();
}

void SqlQueryModel::nextPage()
{
    if (!reloadAvailable)
        return;

    int newPage = this->page + 1;
    if ((newPage + 1) > totalPages)
        newPage = totalPages - 1;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setPage(newPage);
    reloadInternal();
}

void SqlQueryModel::lastPage()
{
    if (!reloadAvailable)
        return;

    int page  = totalPages - 1;
    if (page < 0) // this should never happen, but let's have it just in case
    {
        qWarning() << "Page < 0 while calling SqlQueryModel::lastPage()";
        page = 0;
    }

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setPage(page);
    reloadInternal();
}

void SqlQueryModel::gotoPage(int newPage)
{
    if (!reloadAvailable)
        return;

    if (newPage < 0 || (newPage + 1) > totalPages)
        newPage = 0;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setPage(newPage);
    reloadInternal();
}

bool SqlQueryModel::canReload()
{
    return reloadAvailable;
}

void SqlQueryModel::storeStep1NumbersFromExecution()
{
    lastExecutionTime = queryExecutor->getLastExecutionTime();
    page = queryExecutor->getPage();
    sortOrder = queryExecutor->getSortOrder();

    if (!queryExecutor->getSkipRowCounting())
    {
        rowsAffected = queryExecutor->getRowsAffected();
        totalPages = queryExecutor->getTotalPages();
        if (!queryExecutor->isRowCountingRequired())
            totalRowsReturned = queryExecutor->getTotalRowsReturned();
    }
}

void SqlQueryModel::storeStep2NumbersFromExecution()
{
    if (!queryExecutor->getSkipRowCounting())
    {
        if (queryExecutor->isRowCountingRequired() || rowCount() < CFG_UI.General.NumberOfRowsPerPage.get())
            totalRowsReturned = rowCount();
    }
}

void SqlQueryModel::restoreNumbersToQueryExecutor()
{
    /*
     * Currently only page and sort order have to be restored after failed execution,
     * so reloading current data works on the old page and order, not the ones that were
     * requested but never loaded successfly.
     */
    queryExecutor->setPage(page);
    queryExecutor->setSortOrder(sortOrder);
    emit sortingUpdated(sortOrder);
}

Db* SqlQueryModel::getDb() const
{
    return db;
}

void SqlQueryModel::setDb(Db* value)
{
    db = value;
    queryExecutor->setDb(db);
}

QueryExecutor::SortList SqlQueryModel::getSortOrder() const
{
    return sortOrder;
}

void SqlQueryModel::setSortOrder(const QueryExecutor::SortList& newSortOrder)
{
    sortOrder = newSortOrder;

    if (!reloadAvailable)
        return;

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setSortOrder(newSortOrder);
    reloadInternal();
}

bool SqlQueryModel::wasSchemaModified() const
{
    return queryExecutor->wasSchemaModified();
}

void SqlQueryModel::updateSelectiveCommitRollbackActions(const QItemSelection& selected, const QItemSelection& deselected)
{
    UNUSED(selected);
    UNUSED(deselected);
    QList<SqlQueryItem*> selectedItems = view->getSelectedItems();
    bool result = false;
    if (selectedItems.size() > 0)
    {
        foreach (SqlQueryItem* item, selectedItems)
        {
            if (item->isUncommited())
            {
                result = true;
                break;
            }
        }
    }

    emit selectiveCommitStatusChanged(result);
}

void SqlQueryModel::addNewRowInternal(int rowIdx)
{
    QList<QStandardItem*> items;
    int colCnt = columnCount();
    SqlQueryItem* item;
    for (int i = 0; i < colCnt; i++)
    {
        item = new SqlQueryItem();
        item->setNewRow(true);
        item->setUncommited(true);
        item->setColumn(columns[i].data());
        items << item;
    }
    insertRow(rowIdx, items);

    if (rowIdx == 0) // when adding first row, we need to update header
        updateColumnHeaderLabels();

    view->selectionModel()->clear();;
    view->setCurrentRow(rowIdx);
    view->setFocus();
}

Icon& SqlQueryModel::getIconForIdx(int idx) const
{
    switch (idx)
    {
        case 0:
            return ICONS.SORT_COUNT_01;
        case 1:
            return ICONS.SORT_COUNT_02;
        case 2:
            return ICONS.SORT_COUNT_03;
        case 3:
            return ICONS.SORT_COUNT_04;
        case 4:
            return ICONS.SORT_COUNT_05;
        case 5:
            return ICONS.SORT_COUNT_06;
        case 6:
            return ICONS.SORT_COUNT_07;
        case 7:
            return ICONS.SORT_COUNT_08;
        case 8:
            return ICONS.SORT_COUNT_09;
        case 9:
            return ICONS.SORT_COUNT_10;
        case 10:
            return ICONS.SORT_COUNT_11;
        case 11:
            return ICONS.SORT_COUNT_12;
        case 12:
            return ICONS.SORT_COUNT_13;
        case 13:
            return ICONS.SORT_COUNT_14;
        case 14:
            return ICONS.SORT_COUNT_15;
        case 15:
            return ICONS.SORT_COUNT_16;
        case 16:
            return ICONS.SORT_COUNT_17;
        case 17:
            return ICONS.SORT_COUNT_18;
        case 18:
            return ICONS.SORT_COUNT_19;
        case 19:
            return ICONS.SORT_COUNT_20;
    }
    return ICONS.SORT_COUNT_20_PLUS;
}

void SqlQueryModel::addNewRow()
{
    int row = rowCount();
    SqlQueryItem* currentItem = view->getCurrentItem();
    if (currentItem)
        row = currentItem->index().row();

    addNewRowInternal(row);

    emit commitStatusChanged(true);
}

void SqlQueryModel::addMultipleRows()
{
    bool ok;
    int rows = QInputDialog::getInt(view, tr("Insert multiple rows"), tr("Number of rows to insert:"), 1, 1, 10000, 1, &ok);
    if (!ok)
        return;

    int row = rowCount();
    SqlQueryItem* currentItem = view->getCurrentItem();
    if (currentItem)
        row = currentItem->index().row();

    for (int i = 0; i < rows; i++)
        addNewRowInternal(row);

    emit commitStatusChanged(true);
}

void SqlQueryModel::deleteSelectedRows()
{
    QList<SqlQueryItem*> selectedItems = view->getSelectedItems();
    QSet<int> rows;
    foreach (SqlQueryItem* item, selectedItems)
        rows << item->index().row();

    QList<int> rowList = rows.toList();
    qSort(rowList);

    QList<SqlQueryItem*> newItemsToDelete;
    int cols = columnCount();
    foreach (int row, rowList)
    {
        for (int colIdx = 0; colIdx < cols; colIdx++)
        {
            SqlQueryItem* item = itemFromIndex(row, colIdx);
            if (item->isNewRow())
            {
                newItemsToDelete << item;
                break;
            }

            item->setDeletedRow(true);
            item->setUncommited(true);
        }
    }

    foreach (SqlQueryItem* item, newItemsToDelete)
        removeRow(item->index().row());

    emit commitStatusChanged(getUncommitedItems().size() > 0);
}

void SqlQueryModel::applySqlFilter(const QString& value)
{
    UNUSED(value);
    // For custom query this is not supported.
}

void SqlQueryModel::applyStringFilter(const QString& value)
{
    UNUSED(value);
    // For custom query this is not supported.
}

void SqlQueryModel::applyRegExpFilter(const QString& value)
{
    UNUSED(value);
    // For custom query this is not supported.
}

void SqlQueryModel::resetFilter()
{
    // For custom query this is not supported.
}

int SqlQueryModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return headerColumns.size();
}

QVariant SqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section < 0 || section >= headerColumns.size())
                return QVariant();

            return headerColumns[section];
        }
        else
            return rowNumBase + section;
    }

    if (role == Qt::DecorationRole && orientation == Qt::Horizontal)
    {
        int idx = 0;
        for (const QueryExecutor::Sort& sort : sortOrder)
        {
            if (sort.column == section)
            {
                bool desc = sort.order == QueryExecutor::Sort::DESC;
                return *(getIconForIdx(idx).with(desc ? Icon::SORT_DESC : Icon::SORT_ASC));
            }
            idx++;
        }
        return QVariant();
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

bool SqlQueryModel::isExecutionInProgress() const
{
    return queryExecutor->isExecutionInProgress();
}

void SqlQueryModel::CommitUpdateQueryBuilder::clear()
{
    database.clear();
    table.clear();
    column.clear();
    queryArgs.clear();
    conditions.clear();
}

void SqlQueryModel::CommitUpdateQueryBuilder::setDatabase(const QString& database)
{
    this->database = database;
}

void SqlQueryModel::CommitUpdateQueryBuilder::setTable(const QString& table)
{
    this->table = table;
}

void SqlQueryModel::CommitUpdateQueryBuilder::setColumn(const QString& column)
{
    this->column = column;
}

void SqlQueryModel::CommitUpdateQueryBuilder::setRowId(const RowId& rowId)
{
    static const QString argTempalate = QStringLiteral(":arg%1");

    QString arg;
    QHashIterator<QString,QVariant> it(rowId);
    int i = 0;
    while (it.hasNext())
    {
        it.next();
        arg = argTempalate.arg(i++);
        queryArgs[arg] = it.value();
        conditions << it.key() + " = " + arg;
    }
}

QString SqlQueryModel::CommitUpdateQueryBuilder::build()
{
    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database+".";

    dbAndTable += table;
    return "UPDATE "+dbAndTable+" SET "+column+" = :value WHERE "+conditions.join(" AND ")+";";
}

const QHash<QString, QVariant>&SqlQueryModel::CommitUpdateQueryBuilder::getQueryArgs()
{
    return queryArgs;
}
