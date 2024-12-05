#include "sqlquerymodel.h"
#include "parser/keywords.h"
#include "sqlqueryitem.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "schemaresolver.h"
#include "common/unused.h"
#include "db/sqlerrorcodes.h"
#include "parser/ast/sqlitecreatetable.h"
#include "uiconfig.h"
#include "datagrid/sqlqueryview.h"
#include "services/dbmanager.h"
#include "querygenerator.h"
#include "parser/lexer.h"
#include "common/compatibility.h"
#include "mainwindow.h"
#include "iconmanager.h"
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QMutableListIterator>
#include <QInputDialog>
#include <QTime>
#include <QtMath>
#include <QMessageBox>
#include <QThread>

QSet<SqlQueryModel*> SqlQueryModel::existingModels;

SqlQueryModel::SqlQueryModel(QObject *parent) :
    QStandardItemModel(parent)
{
    queryExecutor = new QueryExecutor();
    queryExecutor->setDataLengthLimit(cellDataLengthLimit);
    connect(queryExecutor, SIGNAL(executionFinished(SqlQueryPtr)), this, SLOT(handleExecFinished(SqlQueryPtr)));
    connect(queryExecutor, SIGNAL(executionFailed(int,QString)), this, SLOT(handleExecFailed(int,QString)));
    connect(queryExecutor, SIGNAL(resultsCountingFinished(quint64,quint64,int)), this, SLOT(resultsCountingFinished(quint64,quint64,int)));

    NotifyManager* notifyManager = NotifyManager::getInstance();
    connect(notifyManager, SIGNAL(objectModified(Db*,QString,QString)), this, SLOT(handlePossibleTableModification(Db*,QString,QString)));
    connect(notifyManager, SIGNAL(objectRenamed(Db*,QString,QString,QString)), this, SLOT(handlePossibleTableRename(Db*,QString,QString,QString)));

    setItemPrototype(new SqlQueryItem());
    existingModels << this;
}

SqlQueryModel::~SqlQueryModel()
{
    existingModels.remove(this);

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

void SqlQueryModel::setParams(const QHash<QString, QVariant>& params)
{
    queryParams = params;
}

void SqlQueryModel::setAsyncMode(bool enabled)
{
    queryExecutor->setAsyncMode(enabled);
}

void SqlQueryModel::executeQuery(bool enforcePage0)
{
    if (queryExecutor->isExecutionInProgress())
    {
        notifyWarn(tr("Only one query can be executed simultaneously."));
        return;
    }

    queryExecutor->setSkipRowCounting(false);
    queryExecutor->setSortOrder(sortOrder);
    queryExecutor->setPage(page > -1 && !enforcePage0 ? page : 0);
    queryExecutor->setForceSimpleMode(simpleExecutionMode);
    reloading = false;

    executeQueryInternal();
}

void SqlQueryModel::executeQueryInternal()
{
    if (!db || !db->isValid())
    {
        notifyWarn(tr("Cannot execute query on undefined or invalid database."));
        internalExecutionStopped();
        return;
    }

    if (isEmptyQuery())
    {
        notifyWarn(tr("Cannot execute empty query."));
        internalExecutionStopped();
        return;
    }

    QList<SqlQueryItem*> uncommittedItems = getUncommittedItems();
    if (uncommittedItems.size() > 0)
    {
        QMessageBox::StandardButton result = QMessageBox::question(nullptr, tr("Uncommitted data"),
                                                                   tr("There are uncommitted data changes. Do you want to proceed anyway? "
                                                                      "All uncommitted changes will be lost."));

        if (result != QMessageBox::Yes)
        {
            internalExecutionStopped();
            return;
        }

        rollback(uncommittedItems);
    }

    emit executionStarted();

    queryExecutor->setQuery(query);
    queryExecutor->setParams(queryParams);
    queryExecutor->setResultsPerPage(getRowsPerPage());
    queryExecutor->setExplainMode(explain);
    queryExecutor->setPreloadResults(true);
    queryExecutor->exec();
}

bool SqlQueryModel::isEmptyQuery() const
{
    if (query.trimmed().isEmpty())
        return true;

    TokenList tokens = Lexer::tokenize(query);
    auto foundIter = std::find_if(tokens.begin(), tokens.end(), [](const TokenPtr& token)
    {
        return token->isMeaningful();
    });

    if (foundIter != tokens.end())
        return false;

    return true;
}

void SqlQueryModel::restoreFocusedCell()
{
    if (!storedFocus.isValid() || getCurrentPage() != storedFocus.forPage || getRowsPerPage() != storedFocus.forRowsPerPage ||
            queryExecutor->getFilters() != storedFocus.forFilter)
    {
        forgetFocusedCell();
        return;
    }

    QModelIndex idx = index(storedFocus.row, storedFocus.column);
    if (idx.isValid())
    {
        view->setCurrentIndex(idx);
        view->scrollTo(idx, QAbstractItemView::EnsureVisible);
    }
}

void SqlQueryModel::internalExecutionStopped()
{
    reloading = false;
    emit loadingEnded(false);
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

void SqlQueryModel::setCellDataLengthLimit(int value)
{
    cellDataLengthLimit = value;
    queryExecutor->setDataLengthLimit(value);
}

QModelIndexList SqlQueryModel::findIndexes(int role, const QVariant& value, int hits) const
{
    QModelIndex startIdx = index(0, 0);
    QModelIndex endIdx = index(rowCount() - 1, columnCount() - 1);
    return findIndexes(startIdx, endIdx, role, value, hits);
}

QModelIndexList SqlQueryModel::findIndexes(const QModelIndex& start, const QModelIndex& end, int role, const QVariant& value, int hits, bool stringApproximation) const
{
    QModelIndexList results;
    bool allHits = hits < 0;
    QModelIndex parentIdx = parent(start);
    int fromRow = start.row();
    int toRow = end.row();
    int fromCol = start.column();
    int toCol = end.column();
    QString stringVal = value.toString();

    for (int row = fromRow; row <= toRow && (allHits || results.count() < hits); row++)
    {
        for (int col = fromCol; col <= toCol && (allHits || results.count() < hits); col++)
        {
            QModelIndex idx = index(row, col, parentIdx);
            if (!idx.isValid())
                 continue;

            QVariant cellVal = data(idx, role);
            if (value != cellVal && !(stringApproximation && stringVal == cellVal.toString()))
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

SqlQueryItem* SqlQueryModel::findAnyInColumn(int column, int role, const QVariant &value) const
{
    QList<SqlQueryItem *> itemList = toItemList(findIndexes(index(0, column), index(rowCount() - 1, column), role, value, 1));
    if (itemList.isEmpty())
        return nullptr;

    return itemList[0];
}

QList<SqlQueryItem*> SqlQueryModel::getUncommittedItems() const
{
    return findItems(SqlQueryItem::DataRole::UNCOMMITTED, true);
}

QList<QList<SqlQueryItem*> > SqlQueryModel::groupItemsByRows(const QList<SqlQueryItem*>& items)
{
    QMap<int,QList<SqlQueryItem*>> itemsByRow;
    for (SqlQueryItem* item : items)
        itemsByRow[item->row()] << item;

    return itemsByRow.values();
}

QHash<AliasedTable, QVector<SqlQueryModelColumn*>> SqlQueryModel::groupColumnsByTable(const QVector<SqlQueryModelColumn*>& columns)
{
    QHash<AliasedTable, QVector<SqlQueryModelColumn*>> columnsByTable;
    AliasedTable table;
    for (SqlQueryModelColumn* col : columns)
    {
        if (!col->column.isNull())
        {
            table.setDatabase(col->database.toLower());
            table.setTable(col->table.toLower());
            table.setTableAlias(col->tableAlias.toLower());
            columnsByTable[table] << col;
        }
        else
            columnsByTable[AliasedTable()] << col;
    }

    return columnsByTable;
}

QHash<AliasedTable, QList<SqlQueryItem*> > SqlQueryModel::groupItemsByTable(const QList<SqlQueryItem*>& items)
{
    QHash<AliasedTable,QList<SqlQueryItem*>> itemsByTable;
    AliasedTable table;
    for (SqlQueryItem* item : items)
    {
        if (item->getColumn())
        {
            table.setDatabase(item->getColumn()->database);
            table.setTable(item->getColumn()->table);
            table.setTableAlias(item->getColumn()->tableAlias);
            itemsByTable[table] << item;
        }
        else
            itemsByTable[AliasedTable()] << item;
    }

    return itemsByTable;
}

QList<SqlQueryItem*> SqlQueryModel::filterOutCommittedItems(const QList<SqlQueryItem*>& items)
{
    // This method doesn't make use of QMutableListIterator to remove items from passed list,
    // because it would require list in argument to drop 'const' keyword and it's already
    // there in calling methods, so it's easier to copy list and filter on the fly.
    QList<SqlQueryItem*> newList;
    for (SqlQueryItem* item : items)
        if (item->isUncommitted())
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
    for (const QModelIndex& idx : indexes)
        list << itemFromIndex(idx);

    return list;
}

void SqlQueryModel::commit()
{
    QList<SqlQueryItem*> items = findItems(SqlQueryItem::DataRole::UNCOMMITTED, true);
    commitInternal(items);
}

void SqlQueryModel::commit(const QList<SqlQueryItem*>& items)
{
    commitInternal(filterOutCommittedItems(items));
}

bool SqlQueryModel::commitRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    const SqlQueryItem* item = itemsInRow.at(0);
    if (!item)
    {
        qWarning() << "null item while call to commitRow() method. It shouldn't happen.";
        return true;
    }
    if (item->isNewRow())
        return commitAddedRow(getRow(item->row()), successfulCommitHandlers); // we need to get all items again, in case of selective commit
    else if (item->isDeletedRow())
        return commitDeletedRow(getRow(item->row()), successfulCommitHandlers); // we need to get all items again, in case of selective commit
    else
        return commitEditedRow(itemsInRow, successfulCommitHandlers);
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

void SqlQueryModel::refreshGeneratedColumns(const QList<SqlQueryItem*>& items)
{
    QHash<SqlQueryItem*, QVariant> resultValues;
    refreshGeneratedColumns(items, resultValues, RowId());
    for (auto resultIt = resultValues.begin(); resultIt != resultValues.end(); resultIt++)
    {
        SqlQueryItem* item = resultIt.key();
        item->setValue(resultIt.value(), true);
        item->setTextAlignment(findValueAlignment(resultIt.value(), item->getColumn()));
    }
}

void SqlQueryModel::refreshGeneratedColumns(const QList<SqlQueryItem*>& items, QHash<SqlQueryItem*, QVariant>& values, const RowId& insertedRowId)
{
    // Find out which columns are generated
    int colIdx = 0;
    QVector<SqlQueryModelColumn*> generatedColumns;
    QHash<SqlQueryModelColumn*, int> generatedColumnIdx;
    for (const SqlQueryModelColumnPtr& column : columns)
    {
        if (column->isGenerated())
        {
            generatedColumns << column.data();
            generatedColumnIdx[column.data()] = colIdx;
        }
        colIdx++;
    }
    if (generatedColumns.isEmpty())
        return;

    QHash<AliasedTable, QVector<SqlQueryModelColumn*>> columnsByTable = groupColumnsByTable(generatedColumns);

    // Filter out deleted items - we won't update generated values for them
    QList<SqlQueryItem*> insertedOrAlteredItems = filter<SqlQueryItem*>(items, [&](SqlQueryItem* item) -> bool
    {
        if (item->isNewRow())
            return !insertedRowId.isEmpty();

        return !item->isDeletedRow();
    });

    SelectCellsQueryBuilder builder;
    QHash<AliasedTable, QList<SqlQueryItem*>> itemsByTable = groupItemsByTable(insertedOrAlteredItems);
    for (auto itemsIt = itemsByTable.begin(); itemsIt != itemsByTable.end(); itemsIt++)
    {
        const AliasedTable table = itemsIt.key();
        QVector<SqlQueryModelColumn*> tableColumns = columnsByTable[table];
        if (tableColumns.isEmpty())
            continue;

        builder.setDatabase(wrapObjIfNeeded(table.getDatabase()));
        builder.setTable(wrapObjIfNeeded(table.getTable()));

        QHash<RowId, QSet<SqlQueryItem*>> itemsPerRowId;
        for (SqlQueryItem* item : itemsIt.value())
        {
            RowId rowId = insertedRowId.isEmpty() ? item->getRowId() : insertedRowId;
            builder.addRowId(rowId);
            for (SqlQueryModelColumn* tableCol : tableColumns)
                itemsPerRowId[rowId] << itemFromIndex(item->row(), generatedColumnIdx[tableCol]);
        }

        for (SqlQueryModelColumn* tableCol : tableColumns)
            builder.addColumn(tableCol->column);

        unite(values, readCellValues(builder, itemsPerRowId));
        builder.clear();
    }
}

QHash<SqlQueryItem*, QVariant> SqlQueryModel::readCellValues(SelectCellsQueryBuilder& queryBuilder, const QHash<RowId, QSet<SqlQueryItem*>>& itemsPerRowId)
{
    QHash<SqlQueryItem*, QVariant> values;

    // Executing query
    SqlQueryPtr results = db->exec(queryBuilder.build(), queryBuilder.getQueryArgs(), Db::Flag::PRELOAD);

    // Handling error
    if (results->isError())
    {
        qCritical() << "Could not load cell values for table" << queryBuilder.getTable() << ", so defaulting them with NULL. Error from database was:"
                    << results->getErrorText();

        for (SqlQueryItem* item : concatSet(itemsPerRowId.values()))
            values[item] = QVariant();

        return values;
    }

    if (!results->hasNext())
    {
        qCritical() << "Could not load cell values for table" << queryBuilder.getTable() << ", so defaulting them with NULL. There were no result rows.";

        for (SqlQueryItem* item : concatSet(itemsPerRowId.values()))
            values[item] = QVariant();

        return values;
    }

    // Reading a row
    while (results->hasNext())
    {
        SqlResultsRowPtr row = results->next();
        if (row->valueList().size() != queryBuilder.getColumnCount())
        {
            qCritical() << "Could not load cell values for table" << queryBuilder.getTable() << ", so defaulting them with NULL. Number of columns from results was invalid:"
                        << row->valueList().size() << ", while expected:" << queryBuilder.getColumnCount();

            for (SqlQueryItem* item : concatSet(itemsPerRowId.values()))
                values[item] = QVariant();

            return values;
        }

        for (SqlQueryItem* item : itemsPerRowId[queryBuilder.readRowId(row)])
            values[item] = row->value(item->getColumn()->column);
    }

    return values;
}

void SqlQueryModel::rollback()
{
    QList<SqlQueryItem*> items = findItems(SqlQueryItem::DataRole::UNCOMMITTED, true);
    rollbackInternal(items);
}

void SqlQueryModel::rollback(const QList<SqlQueryItem*>& items)
{
    rollbackInternal(filterOutCommittedItems(items));
}

void SqlQueryModel::commitInternal(const QList<SqlQueryItem*>& items)
{
    Db* db = getDb();
    if (!db->isOpen())
    {
        notifyError(tr("Cannot commit the data for a cell that refers to the already closed database."));
        return;
    }

    attachDependencyTables();

    if (!db->begin())
    {
        notifyError(tr("Could not begin transaction on the database. Details: %1").arg(db->getErrorText()));
        return;
    }

    // Getting number of rows to be added and deleted, so we can update totalPages at the end
    int numberOfItemsAdded = groupItemsByRows(findItems(SqlQueryItem::DataRole::NEW_ROW, true)).size();
    int numberOfItemsDeleted = groupItemsByRows(findItems(SqlQueryItem::DataRole::DELETED, true)).size();

    // Removing "commit error" mark from items that are going to be committed now
    for (SqlQueryItem* item : items)
        item->setCommittingError(false);

    // Grouping by row and committing
    QList<QList<SqlQueryItem*>> groupedItems = groupItemsByRows(items);
    emit aboutToCommit(groupedItems.size());

    int step = 1;
    rowsDeletedSuccessfullyInTheCommit.clear();
    QList<CommitSuccessfulHandler> successfulCommitHandlers; // list of lambdas to execute after all rows were committed successfully
    bool ok = true;
    for (const QList<SqlQueryItem*>& itemsInRow : groupedItems)
    {
        if (!commitRow(itemsInRow, successfulCommitHandlers))
            ok = false;

        emit committingStepFinished(step++);
    }

    // Getting current uncommitted list (after rows deletion it may be different)
    QList<SqlQueryItem*> itemsLeft = findItems(SqlQueryItem::DataRole::UNCOMMITTED, true);

    // Getting common elements of initial and current item list, because of a possibility of the selective commit
    QMutableListIterator<SqlQueryItem*> it(itemsLeft);
    while (it.hasNext())
    {
        if (!items.contains(it.next()))
            it.remove();
    }

    // Committing to the database
    if (ok)
    {
        if (!db->commit())
        {
            ok = false;
            notifyError(tr("An error occurred while committing the transaction: %1").arg(db->getErrorText()));
        }
        else
        {
            // Call all successfull commit handler to refresh cell metadata, etc.
            for (CommitSuccessfulHandler& handler : successfulCommitHandlers)
                handler();

            // Refresh generated columns of altered rows
            refreshGeneratedColumns(itemsLeft);

            // Committed successfully
            for (SqlQueryItem* item : itemsLeft)
            {
                item->setUncommitted(false);
                item->setNewRow(false);
            }

            // Physically delete rows
            std::sort(rowsDeletedSuccessfullyInTheCommit.begin(), rowsDeletedSuccessfullyInTheCommit.end());
            int removeOffset = 0;
            for (int row : rowsDeletedSuccessfullyInTheCommit)
                removeRow(row - removeOffset++); // deleting row decrements all rows below

            emit commitStatusChanged(getUncommittedItems().size() > 0);
        }
    }
    rowsDeletedSuccessfullyInTheCommit.clear();

    if (!ok)
    {
        if (!db->rollback())
        {
            notifyError(tr("An error occurred while rolling back the transaction: %1").arg(db->getErrorText()));
            // Nothing else we can do about it, but it should not happen.
        }
    }

    detachDependencyTables();

    // Updating added/deleted counts, to honor rows not deleted because of some errors
    numberOfItemsAdded -= groupItemsByRows(findItems(SqlQueryItem::DataRole::NEW_ROW, true)).size();
    numberOfItemsDeleted -= groupItemsByRows(findItems(SqlQueryItem::DataRole::DELETED, true)).size();
    int itemsAddedDeletedDelta = numberOfItemsAdded - numberOfItemsDeleted;

    recalculateRowsAndPages(itemsAddedDeletedDelta);

    emit commitFinished();
}

void SqlQueryModel::rollbackInternal(const QList<SqlQueryItem*>& items)
{
    QList<QList<SqlQueryItem*> > groupedItems = groupItemsByRows(items);
    for (const QList<SqlQueryItem*>& itemsInRow : groupedItems)
        rollbackRow(itemsInRow);

    emit commitStatusChanged(getUncommittedItems().size() > 0);
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

StrHash<QString> SqlQueryModel::attachDependencyTables()
{
    dbNameToAttachNameMapForCommit.clear();
    dbListToDetach.clear();

    QString attachName;
    for (const QString& reqAttach : queryExecutor->getRequiredDbAttaches())
    {
        Db* attachDb = DBLIST->getByName(reqAttach, Qt::CaseInsensitive);
        if (!attachDb)
        {
            qCritical() << "Could not resolve database" << reqAttach << ", while it's a required attach name for SqlQueryModel to commit edited data!"
                        << "This may result in errors when committing some data modifications.";
            continue;
        }

        attachName = db->attach(attachDb);
        if (attachName.isNull())
        {
            qCritical() << "Could not attach database" << reqAttach << ", while it's a required attach name for SqlQueryModel to commit edited data!"
                        << "This may result in errors when committing some data modifications.";
            continue;
        }

        dbNameToAttachNameMapForCommit[reqAttach] = attachName;
        dbListToDetach << attachDb;
    }

    return dbNameToAttachNameMapForCommit;
}

void SqlQueryModel::detachDependencyTables()
{
    for (Db* dbToDetach : dbListToDetach)
        db->detach(dbToDetach);

    dbNameToAttachNameMapForCommit.clear();
    dbListToDetach.clear();
}

void SqlQueryModel::rememberFocusedCell()
{
    QModelIndex idx = getView()->currentIndex();
    storedFocus.row = idx.row();
    storedFocus.column = idx.column();
    storedFocus.forPage = getCurrentPage();
    storedFocus.forRowsPerPage = getRowsPerPage();
    storedFocus.forFilter = queryExecutor->getFilters();
}

void SqlQueryModel::forgetFocusedCell()
{
    storedFocus.reset();
}

QString SqlQueryModel::generateSelectQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    BiStrHash attachMap = BiStrHash(attachDependencyTables().toQHash());
    QString sql = generator.generateSelectFromSelect(db, getQuery(), values, attachMap);
    detachDependencyTables();

    return sql;
}

QString SqlQueryModel::generateSelectFunctionQueryForItems(const QString& function, const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);
    QStringList orderedColumns = toOrderedColumnNames(items);

    QueryGenerator generator;
    QString sql = generator.generateSelectFunction(function, orderedColumns, values);

    return sql;
}

QString SqlQueryModel::generateInsertQueryForItems(const QList<SqlQueryItem*>& items)
{
    UNUSED(items);
    return QString();
}

QString SqlQueryModel::generateUpdateQueryForItems(const QList<SqlQueryItem*>& items)
{
    UNUSED(items);
    return QString();
}

QString SqlQueryModel::generateDeleteQueryForItems(const QList<SqlQueryItem*>& items)
{
    UNUSED(items);
    return QString();
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

bool SqlQueryModel::commitAddedRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    UNUSED(itemsInRow);
    UNUSED(successfulCommitHandlers);
    return false;
}

bool SqlQueryModel::commitEditedRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    if (itemsInRow.size() == 0)
    {
        qWarning() << "SqlQueryModel::commitEditedRow() called with no items in the list.";
        return true;
    }

    QHash<AliasedTable,QList<SqlQueryItem*>> itemsByTable = groupItemsByTable(itemsInRow);

    // Values
    QString query;
    SqlQueryModelColumn* col = nullptr;
    QHash<QString,QVariant> queryArgs;
    QStringList assignmentArgs;
    RowId rowId;
    RowId newRowId;
    CommitUpdateQueryBuilder queryBuilder;
    QHashIterator<AliasedTable,QList<SqlQueryItem*>> it(itemsByTable);
    QList<SqlQueryItem*> items;
    AliasedTable table;
    while (it.hasNext())
    {
        it.next();
        table = it.key();
        if (table.getTable().isNull())
        {
            qCritical() << "Tried to commit null table in SqlQueryModel::commitEditedRow().";
            continue;
        }

        items = it.value();
        if (items.size() == 0)
            continue;

        // RowId
        queryBuilder.clear();
        rowId = items.first()->getRowId();
        queryBuilder.setRowId(rowId);
        newRowId = getNewRowId(rowId, items); // if any of item updates any of rowid columns, then this will be different than initial rowid

        // Database and table
        queryBuilder.setTable(wrapObjIfNeeded(table.getTable()));
        if (!table.getDatabase().isNull())
        {
            QString tableDb = getDatabaseForCommit(table.getDatabase());
            queryBuilder.setDatabase(wrapObjIfNeeded(tableDb));
        }

        for (SqlQueryItem* item : items)
        {
            col = item->getColumn();
            if (col->editionForbiddenReason.size() > 0 || item->isJustInsertedWithOutRowId())
            {
                QString errMsg = tr("Tried to commit a cell which is not editable (yet modified and waiting for commit)! This is a bug. Please report it.");
                item->setCommittingError(true, errMsg);
                notifyError(errMsg);
                return false;
            }

            // Column
            queryBuilder.addColumn(wrapObjIfNeeded(col->column));
        }

        // Completing query
        query = queryBuilder.build();

        // RowId condition arguments
        queryArgs = queryBuilder.getQueryArgs();

        // Per-column arguments
        assignmentArgs = queryBuilder.getAssignmentArgs();
        for (int i = 0, total = items.size(); i < total; ++i)
            queryArgs[assignmentArgs[i]] = items[i]->getValue();

        // Get the data
        SqlQueryPtr results = db->exec(query, queryArgs);
        if (results->isError())
        {
            QString errMsg = tr("An error occurred while committing the data: %1").arg(results->getErrorText());
            for (SqlQueryItem* item : items)
                item->setCommittingError(true, errMsg);

            notifyError(errMsg);
            return false;
        }

        // After successful commit, check if RowId was modified and upadate it accordingly
        if (rowId != newRowId)
        {
            // ...and do it with deferred lambda, so only after all rows were successully committed
            successfulCommitHandlers << [this, table, rowId, newRowId]()
            {
                updateRowIdForAllItems(table, rowId, newRowId);
            };
        }
    }

    return true;
}

bool SqlQueryModel::commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow, QList<SqlQueryModel::CommitSuccessfulHandler>& successfulCommitHandlers)
{
    UNUSED(successfulCommitHandlers);
    if (itemsInRow.size() == 0)
    {
        qCritical() << "No items passed to SqlQueryModel::commitDeletedRow().";
        return false;
    }

    int row = itemsInRow[0]->index().row();
    rowsDeletedSuccessfullyInTheCommit << row;
    return true;
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
    for (SqlQueryItem* item : itemsInRow)
        item->rollback();
}

void SqlQueryModel::rollbackDeletedRow(const QList<SqlQueryItem*>& itemsInRow)
{
    for (SqlQueryItem* item : itemsInRow)
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
    for (SqlQueryModelColumnPtr modelColumn : columns)
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

bool SqlQueryModel::loadData(SqlQueryPtr results)
{
    if (rowCount() > 0)
        clear();

    allDataLoaded = false;
    view->horizontalHeader()->show();

    // Read columns first. It will be needed later.
    bool rowsLimited = readColumns();

    // Load data
    SqlResultsRowPtr row;
    int rowIdx = 0;
    int rowsPerPage = getRowsPerPage();
    rowNumBase = getCurrentPage() * rowsPerPage + 1;

    updateColumnHeaderLabels();
    QList<QList<QStandardItem*>> rowList;
    while (results->hasNext() && rowIdx < rowsPerPage)
    {
        row = results->next();
        if (!row)
            break;

        rowList << loadRow(row, results);

        if ((rowIdx % 50) == 0)
        {
            qApp->processEvents();
            if (!existingModels.contains(this))
                return false;
        }

        rowIdx++;
    }

    if (rowsLimited && rowIdx >= columnRatioBasedRowLimit)
    {
        NOTIFY_MANAGER->info(tr("Number of rows per page was decreased to %1 due to number of columns (%2) in the data view.")
                             .arg(columnRatioBasedRowLimit).arg(columns.size()));
    }

    rowIdx = 0;
    for (const QList<QStandardItem*>& row : rowList)
        insertRow(rowIdx++, row);

    allDataLoaded = true;
    return true;
}

QList<QStandardItem*> SqlQueryModel::loadRow(SqlResultsRowPtr row, SqlQueryPtr results)
{
    QStringList columnNames = results->getColumnNames();
    BiStrHash typeColumnToResColumn = queryExecutor->getTypeColumns();

    QList<QStandardItem*> itemList;
    SqlQueryItem* item = nullptr;
    RowId rowId;
    int colIdx = 0;
    for (const QVariant& value : row->valueList().mid(0, resultColumnCount))
    {
        item = new SqlQueryItem();
        rowId = getRowIdValue(row, colIdx);
        updateItem(item, value, colIdx, rowId, row, columnNames, typeColumnToResColumn);
        itemList << item;
        colIdx++;
    }

    return itemList;
}

RowId SqlQueryModel::getRowIdValue(SqlResultsRowPtr row, int columnIdx)
{
    RowId rowId;
    AliasedTable table = tablesForColumns[columnIdx];
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


void SqlQueryModel::updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId, SqlResultsRowPtr row,
                               const QStringList& columnNames, const BiStrHash& typeColumnToResColumn)
{
    if (columnIndex >= columnNames.size())
    {
        updateItem(item, value, columnIndex, rowId);
        return;
    }

    QString colName = columnNames[columnIndex];
    if (typeColumnToResColumn.isEmpty() || !typeColumnToResColumn.containsRight(colName))
    {
        updateItem(item, value, columnIndex, rowId);
        return;
    }

    QString colTypeColumnName = typeColumnToResColumn.valueByRight(colName);
    QString colTypeStr = row->value(colTypeColumnName).toString();
    SqliteDataType sqliteDataType = toSqliteDataType(colTypeStr);

    switch (sqliteDataType)
    {
        case SqliteDataType::INTEGER:
        case SqliteDataType::REAL:
            updateItem(item, value, columnIndex, rowId, Qt::AlignRight);
            break;
        case SqliteDataType::_NULL:
        case SqliteDataType::TEXT:
        case SqliteDataType::BLOB:
            updateItem(item, value, columnIndex, rowId, Qt::AlignLeft);
            break;
        case SqliteDataType::UNKNOWN:
            updateItem(item, value, columnIndex, rowId);
            break;
    }
}

void SqlQueryModel::updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId)
{
    SqlQueryModelColumnPtr column = columns[columnIndex];
    Qt::Alignment alignment = findValueAlignment(value, column.data());
    updateItem(item, value, columnIndex, rowId, alignment);
}

void SqlQueryModel::updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId, Qt::Alignment alignment)
{
    SqlQueryModelColumnPtr column = columns[columnIndex];
    item->setJustInsertedWithOutRowId(false);
    item->setValue(value, true);
    item->setColumn(column.data());
    item->setTextAlignment(alignment);
    item->setRowId(rowId);
}

Qt::Alignment SqlQueryModel::findValueAlignment(const QVariant& value, SqlQueryModelColumn* column)
{
    if ((column->isNumeric() || column->isNull()) && isNumeric(value))
        return Qt::AlignRight|Qt::AlignVCenter;
    else
        return Qt::AlignLeft|Qt::AlignVCenter;
}

RowId SqlQueryModel::getNewRowId(const RowId& currentRowId, const QList<SqlQueryItem*> items)
{
    if (currentRowId.size() > 1)
    {
        // For WITHOUT ROWID tables we need to look up all columns
        QStringList rowIdColumns = currentRowId.keys();
        RowId newRowIdCandidate = currentRowId;
        int idx;
        for (SqlQueryItem* item : items)
        {
            if (rowIdColumns.contains(item->getColumn()->column, Qt::CaseInsensitive))
            {
                idx = indexOf(rowIdColumns, item->getColumn()->column, Qt::CaseInsensitive);
                newRowIdCandidate[rowIdColumns[idx]] = item->getValue();
            }
        }
        return newRowIdCandidate;
    }
    else
    {
        // Check for an update on the standard ROWID
        SqlQueryModelColumn* col = nullptr;
        for (SqlQueryItem* item : items)
        {
            col = item->getColumn();
            QStringList tableRowIdColumns = tableToRowIdColumn[col->getAliasedTable()].values();
            if (tableRowIdColumns.contains(col->column, Qt::CaseInsensitive))
            {
                RowId newRowId;
                newRowId[col->column] = item->getValue();
                return newRowId;
            }

            if (isRowIdKeyword(col->column) || col->isRowIdPk())
            {
                RowId newRowId;
                newRowId["ROWID"] = item->getValue();
                return newRowId;
            }
        }
    }

    return currentRowId;
}

void SqlQueryModel::updateRowIdForAllItems(const AliasedTable& table, const RowId& rowId, const RowId& newRowId)
{
    SqlQueryItem* item = nullptr;
    for (int row = 0; row < rowCount(); row++)
    {
        for (int col = 0; col < columnCount(); col++)
        {
            item = itemFromIndex(row, col);
            if (item->getColumn()->database.compare(table.getDatabase(), Qt::CaseInsensitive) != 0)
                continue;

            if (item->getColumn()->table.compare(table.getTable(), Qt::CaseInsensitive) != 0)
                continue;

            if (item->getRowId() != rowId)
                continue;

            item->setRowId(newRowId);
        }
    }
}

QHash<QString, QVariantList> SqlQueryModel::toValuesGroupedByColumns(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values;
    for (SqlQueryItem* item : items)
        values[item->getColumn()->displayName] << item->getValue();

    return values;
}

QStringList SqlQueryModel::toOrderedColumnNames(const QList<SqlQueryItem*>& items)
{
    QStringList cols;
    int row = -1;
    QMap<int,QList<SqlQueryItem*>> itemsByRow;
    for (SqlQueryItem* item : items)
    {
        if (row != -1 && item->row() != row)
            break;
        row = item->row();
        cols << item->getColumn()->displayName;
    }

    return cols;
}

bool SqlQueryModel::supportsModifyingQueriesInMenu() const
{
    return false;
}

bool SqlQueryModel::readColumns()
{
    columns.clear();
    tableToRowIdColumn.clear();
    tablesInUse.clear();

    // Reading column mapping for ROWID columns
    AliasedTable aliasedTable;
    DbAndTable dbAndTable;
    for (const QueryExecutor::ResultRowIdColumnPtr& resCol : queryExecutor->getRowIdResultColumns())
    {
        if (resCol->dbName.isEmpty() || resCol->dbName.toLower() == "main" || resCol->dbName.toLower() == "temp")
            dbAndTable.setDb(db);
        else if (!resCol->dbName.isEmpty())
            dbAndTable.setDb(DBLIST->getByName(resCol->dbName));

        dbAndTable.setDatabase(resCol->database);
        dbAndTable.setTable(resCol->table);
        tablesInUse << dbAndTable;

        aliasedTable.setDatabase(resCol->dbName);
        aliasedTable.setTable(resCol->table);
        aliasedTable.setTableAlias(resCol->tableAlias);
        tableToRowIdColumn[aliasedTable] = resCol->queryExecutorAliasToColumn;
    }

    // Reading column details (datatype, constraints)
    readColumnDetails();

    // Preparing other usful information about columns
    resultColumnCount = queryExecutor->getResultColumns().size();
    tablesForColumns = getTablesForColumns();
    columnEditionStatus = getColumnEditionEnabledList();

    // Rows limit to avoid out of memory problems
    columnRatioBasedRowLimit = -1;
    int rowsPerPage = getRowsPerPage();
    if (!columns.isEmpty() && CFG_UI.General.LimitRowsForManyColumns.get())
        columnRatioBasedRowLimit = 50000 / columns.size();

    bool rowsLimited = (columnRatioBasedRowLimit > -1 && columnRatioBasedRowLimit < rowsPerPage);

    // We have fresh info about columns
    structureOutOfDate = false;

    return rowsLimited;
}

void SqlQueryModel::readColumnDetails()
{
    // Preparing global (table oriented) edition forbidden reasons
    QSet<SqlQueryModelColumn::EditionForbiddenReason> editionForbiddenGlobalReasons;
    for (QueryExecutor::EditionForbiddenReason reason : queryExecutor->getEditionForbiddenGlobalReasons())
        editionForbiddenGlobalReasons << SqlQueryModelColumn::convert(reason);

    // Reading all the details from query executor source tables
    QHash<AliasedTable, TableDetails> tableDetails = readTableDetails();

    // Preparing for processing
    AliasedTable table;
    Column column;
    TableDetails details;
    TableDetails::ColumnDetails colDetails;

    SqlQueryModelColumnPtr modelColumn;
    SqliteColumnTypePtr modelColumnType;
    SqlQueryModelColumn::Constraint* modelConstraint = nullptr;

    for (const QueryExecutor::ResultColumnPtr& resCol : queryExecutor->getResultColumns())
    {
        // Creating new column for the model (this includes column oriented forbidden reasons)
        modelColumn = SqlQueryModelColumnPtr::create(resCol);

        // Adding global edition forbidden reasons
        modelColumn->editionForbiddenReason += editionForbiddenGlobalReasons;

        // Getting details of given table and column
        table = AliasedTable(modelColumn->database, modelColumn->table, modelColumn->tableAlias);
        column = Column(modelColumn->database, modelColumn->table, modelColumn->column);

        details = tableDetails[table];
        colDetails = details.columns[modelColumn->column];

        // Column type
        modelColumnType = colDetails.type;
        if (modelColumnType)
            modelColumn->dataType = DataType(modelColumnType->name, modelColumnType->precision, modelColumnType->scale);

        // Column constraints
        for (SqliteCreateTable::Column::ConstraintPtr constrPtr : colDetails.constraints)
        {
            modelConstraint = SqlQueryModelColumn::Constraint::create(constrPtr);
            if (modelConstraint)
                modelColumn->constraints << modelConstraint;

            modelColumn->postProcessConstraints();
        }

        // Table constraints
        for (SqliteCreateTable::ConstraintPtr constrPtr : details.constraints)
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

QHash<AliasedTable, SqlQueryModel::TableDetails> SqlQueryModel::readTableDetails()
{
    QHash<AliasedTable, TableDetails> results;
    SqliteQueryPtr query;
    SqliteCreateTablePtr createTable;
    SchemaResolver resolver(getDb());
    QString database;
    AliasedTable table;
    QString columnName;

    for (const QueryExecutor::SourceTablePtr& srcTable : queryExecutor->getSourceTables())
    {
        database = srcTable->database.isEmpty() ? "main" : srcTable->database;

        query = resolver.getParsedObject(database, srcTable->table, SchemaResolver::TABLE);
        if (!query || !query.dynamicCast<SqliteCreateTable>())
        {
            qWarning() << "Could not get parsed table while reading table details in SqlQueryModel. Queried table was:"
                       << database + "." + srcTable->table;
            continue;
        }
        createTable = query.dynamicCast<SqliteCreateTable>();

        // Table details
        TableDetails tableDetails;
        table = {database, srcTable->table, srcTable->alias};

        // Table constraints
        for (SqliteCreateTable::Constraint* tableConstr : createTable->constraints)
            tableDetails.constraints << tableConstr->detach<SqliteCreateTable::Constraint>();

        // Table columns
        for (SqliteCreateTable::Column* columnStmt : createTable->columns)
        {
            // Column details
            TableDetails::ColumnDetails columnDetails;
            columnName = stripObjName(columnStmt->name);

            // Column type
            if (columnStmt->type)
                columnDetails.type = columnStmt->type->detach<SqliteColumnType>();
            else
                columnDetails.type = SqliteColumnTypePtr();

            // Column constraints
            for (SqliteCreateTable::Column::Constraint* columnConstr : columnStmt->constraints)
                columnDetails.constraints << columnConstr->detach<SqliteCreateTable::Column::Constraint>();

            tableDetails.columns[columnName] = columnDetails;
        }

        results[table] = tableDetails;
    }

    return results;

}

QList<AliasedTable> SqlQueryModel::getTablesForColumns()
{
    QList<AliasedTable> columnTables;
    AliasedTable table;
    for (SqlQueryModelColumnPtr column : columns)
    {
        if (column->editionForbiddenReason.size() > 0)
        {
            columnTables << AliasedTable();
            continue;
        }
        table = AliasedTable(column->database, column->table, column->tableAlias);
        columnTables << table;
    }
    return columnTables;
}

QList<bool> SqlQueryModel::getColumnEditionEnabledList()
{
    QList<bool> columnEditionEnabled;
    for (SqlQueryModelColumnPtr column : columns)
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
    for (SqlQueryModelColumnPtr column : columns)
    {
        headerColumns << column->displayName;
    }

    setColumnCount(headerColumns.size());
}

void SqlQueryModel::handleExecFinished(SqlQueryPtr results)
{
    if (results->isError())
    {
        emit executionFailed(tr("Error while executing SQL query on database '%1': %2").arg(db->getName(), results->getErrorText()));
        return;
    }

    emit aboutToLoadResults();
    storeStep1NumbersFromExecution();
    if (!loadData(results))
        return;

    storeStep2NumbersFromExecution();

    requiredDbAttaches = queryExecutor->getRequiredDbAttaches();
    reloadAvailable = true;

    emit loadingEnded(true);
    restoreNumbersToQueryExecutor();
    if (!reloading)
        emit executionSuccessful();

    reloading = false;

    bool rowsCountedManually = queryExecutor->isRowCountingRequired() || rowCount() < getRowsPerPage();
    bool countRes = false;
    if (rowsCountedManually)
    {
        emit totalRowsAndPagesAvailable();
        emit storeExecutionInHistory();
    }
    else
        countRes = queryExecutor->countResults();

    if (!countRes || !queryExecutor->getAsyncMode())
    {
        results.clear();
        detachDatabases();
    }
    restoreFocusedCell();
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
        emit executionFailed(tr("Error while executing SQL query on database '%1': %2").arg(db->getName(), errorMessage));

    restoreNumbersToQueryExecutor();
    resultsCountingFinished(0, 0, 0);

    reloading = false;
}

void SqlQueryModel::resultsCountingFinished(quint64 rowsAffected, quint64 rowsReturned, int totalPages)
{
    // TotalPages provided by QueryExecutor is wrong if there are tons of columns in results, as row limit is applied
    // to prevent memory exhaustion. QueryExecutor is not aware of the limit at the moment of execution, so it calculates
    // total number of pages incorrectly.
    UNUSED(totalPages);

    this->rowsAffected = rowsAffected;
    this->totalRowsReturned = rowsReturned;
    this->totalPages = (int)qCeil(((double)totalRowsReturned) / ((double)getRowsPerPage()));
    detachDatabases();
    emit totalRowsAndPagesAvailable();
    emit storeExecutionInHistory();
}

void SqlQueryModel::itemValueEdited(SqlQueryItem* item)
{
    UNUSED(item);
    emit commitStatusChanged(getUncommittedItems().size() > 0);
}

void SqlQueryModel::repaintAllItems()
{
    QModelIndex startIdx = index(0, 0);
    if (!startIdx.isValid())
        return;

    QModelIndex endIdx = index(rowCount() - 1, columnCount() - 1);
    emit dataChanged(startIdx, endIdx, QVector<int>({Qt::DisplayRole, Qt::EditRole}));
}

void SqlQueryModel::changeSorting(int logicalIndex, Qt::SortOrder order)
{
    if (!reloadAvailable)
        return;

    QueryExecutor::SortList sortList = QueryExecutor::SortList();
    if (logicalIndex > -1)
        sortList = {QueryExecutor::Sort(order, logicalIndex)};

    queryExecutor->setSkipRowCounting(true);
    queryExecutor->setSortOrder(sortList);
    reloadInternal();
}

void SqlQueryModel::changeSorting(int logicalIndex)
{
    Qt::SortOrder newOrder = Qt::AscendingOrder;
    if (sortOrder.size() != 1)
    {
        changeSorting(logicalIndex, newOrder);
        return;
    }

    QueryExecutor::Sort singleOrder = sortOrder.first();
    if (singleOrder.column != logicalIndex)
    {
        changeSorting(logicalIndex, newOrder);
        return;
    }

    switch (singleOrder.order)
    {
        case QueryExecutor::Sort::ASC:
            newOrder = Qt::DescendingOrder;
            break;
        case QueryExecutor::Sort::DESC:
            logicalIndex = -1;
            break;
        case QueryExecutor::Sort::NONE:
            newOrder = Qt::AscendingOrder;
            break;
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

    QueryExecutor::SortList newSortOrder = queryExecutor->getSortOrder();
    if (!sortOrder.isEmpty() && newSortOrder.isEmpty())
        notifyWarn(tr("There are less columns in the new query, sort order has been reset."));
    sortOrder = newSortOrder;
    rowsAffected = queryExecutor->getRowsAffected();

    if (!queryExecutor->getSkipRowCounting())
    {
        if (!queryExecutor->isRowCountingRequired())
            totalRowsReturned = queryExecutor->getTotalRowsReturned();

        totalPages = (int)qCeil(((double)totalRowsReturned) / ((double)getRowsPerPage()));
    }
}

void SqlQueryModel::storeStep2NumbersFromExecution()
{
    if (!queryExecutor->getSkipRowCounting())
    {
        if (queryExecutor->isRowCountingRequired() || rowCount() < getRowsPerPage())
            totalRowsReturned = rowCount();
    }
}

void SqlQueryModel::restoreNumbersToQueryExecutor()
{
    /*
     * Currently only page and sort order have to be restored after failed execution,
     * so reloading current data works on the old page and order, not the ones that were
     * requested but never loaded successfully.
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

bool SqlQueryModel::wasDataModifyingQuery() const
{
    return queryExecutor->wasDataModifyingQuery();
}

void SqlQueryModel::updateSelectiveCommitRollbackActions(const QItemSelection& selected, const QItemSelection& deselected)
{
    UNUSED(selected);
    UNUSED(deselected);
    QList<SqlQueryItem*> selectedItems = view->getSelectedItems();
    bool result = false;
    if (selectedItems.size() > 0)
    {
        for (SqlQueryItem* item : selectedItems)
        {
            if (item->isUncommitted())
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
    SqlQueryItem* item = nullptr;
    SqlQueryModelColumn* columnModel = nullptr;
    for (int i = 0; i < colCnt; i++)
    {
        columnModel = columns[i].data();

        item = new SqlQueryItem();
        item->setNewRow(true);
        item->setUncommitted(true);
        item->setColumn(columnModel);

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

void SqlQueryModel::detachDatabases()
{
    queryExecutor->releaseResultsAndCleanup();
}

QString SqlQueryModel::getDatabaseForCommit(const QString& database)
{
    if (dbNameToAttachNameMapForCommit.contains(database, Qt::CaseInsensitive))
        return dbNameToAttachNameMapForCommit[database];

    return database;
}

void SqlQueryModel::recalculateRowsAndPages(int rowsDelta)
{
    totalRowsReturned += rowsDelta;

    int rowsPerPage = getRowsPerPage();
    totalPages = (int)qCeil(((double)totalRowsReturned) / ((double)rowsPerPage));
    emit totalRowsAndPagesAvailable();

    if (rowCount() == 0)
        reload();
}

int SqlQueryModel::getInsertRowIndex()
{
    Cfg::InsertRowPlacement placement = static_cast<Cfg::InsertRowPlacement>(CFG_UI.General.InsertRowPlacement.get());

    int row = rowCount();
    if (placement == Cfg::AT_THE_END)
        return row;

    SqlQueryItem* currentItem = view->getCurrentItem();
    if (!currentItem)
        return row;

    row = currentItem->index().row();
    if (placement == Cfg::AFTER_CURRENT)
        row++;

    return row;
}

void SqlQueryModel::notifyItemEditionEnded(const QModelIndex& idx)
{
    emit itemEditionEnded(itemFromIndex(idx));
}

int SqlQueryModel::getRowsPerPage() const
{
    int rowsPerPage = CFG_UI.General.NumberOfRowsPerPage.get();
    if (hardRowLimit > -1)
        rowsPerPage = hardRowLimit;

    if (CFG_UI.General.LimitRowsForManyColumns.get() && columnRatioBasedRowLimit > -1 && columnRatioBasedRowLimit < rowsPerPage)
        rowsPerPage = columnRatioBasedRowLimit;

    return rowsPerPage;
}

int SqlQueryModel::getQueryCountLimitForSmartMode() const
{
    return queryExecutor->getQueryCountLimitForSmartMode();
}

void SqlQueryModel::setQueryCountLimitForSmartMode(int value)
{
    queryExecutor->setQueryCountLimitForSmartMode(value);
}

void SqlQueryModel::insertCustomRow(const QList<QVariant> &values, int insertionIndex)
{
    SqlQueryItem* cellItem = nullptr;
    int colIdx = 0;
    QList<QStandardItem*> row;
    for (const QVariant& value : values)
    {
        cellItem = new SqlQueryItem();
        updateItem(cellItem, value, colIdx++, RowId());
        row << cellItem;
    }
    insertRow(insertionIndex, row);
}

void SqlQueryModel::setDesiredColumnWidth(int colIdx, int width)
{
    SqlQueryModelColumnPtr columnModel = columns[colIdx];
    if (!columnModel)
    {
        qWarning() << "Missing column model for column with index" << colIdx << "while resizing column.";
        return;
    }

    AliasedColumn column(columnModel->database, columnModel->table, columnModel->column, columnModel->displayName);
    columnWidths[column] = width;
}

int SqlQueryModel::getDesiredColumnWidth(int colIdx)
{
    SqlQueryModelColumnPtr columnModel = columns[colIdx];
    if (!columnModel)
        return -1;

    AliasedColumn column(columnModel->database, columnModel->table, columnModel->column, columnModel->displayName);
    if (!columnWidths.contains(column))
        return -1;

    return columnWidths[column];
}

bool SqlQueryModel::isStructureOutOfDate() const
{
    return structureOutOfDate;
}

bool SqlQueryModel::isAllDataLoaded() const
{
    return allDataLoaded;
}

int SqlQueryModel::getHardRowLimit() const
{
    return hardRowLimit;
}

void SqlQueryModel::setHardRowLimit(int value)
{
    hardRowLimit = value;
}

bool SqlQueryModel::getSimpleExecutionMode() const
{
    return simpleExecutionMode;
}

void SqlQueryModel::setSimpleExecutionMode(bool value)
{
    simpleExecutionMode = value;
}

void SqlQueryModel::addNewRow()
{
    addNewRowInternal(getInsertRowIndex());

    emit commitStatusChanged(true);
}

void SqlQueryModel::addMultipleRows()
{
    bool ok;
    int rows = QInputDialog::getInt(view, tr("Insert multiple rows"), tr("Number of rows to insert:"), 1, 1, 10000, 1, &ok);
    if (!ok)
        return;

    int row = getInsertRowIndex();
    for (int i = 0; i < rows; i++)
        addNewRowInternal(row++);

    emit commitStatusChanged(true);
}

void SqlQueryModel::deleteSelectedRows()
{
    QList<SqlQueryItem*> selectedItems = view->getSelectedItems();
    QSet<int> rows;
    QSet<int> newRows;
    for (SqlQueryItem* item : selectedItems)
    {
        int row = item->index().row();
        if (item->isNewRow())
            newRows << row;

        rows << row;
    }

    QList<int> rowList = rows.values();
    QList<int> newRowList = newRows.values();
    sSort(rowList);
    sSort(newRowList);

    QList<SqlQueryItem*> newItemsToDelete;
    int cols = columnCount();
    for (int row : rowList)
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
            item->setUncommitted(true);
        }
    }

    if (newItemsToDelete.size() > 0)
    {
        QStringList rowNumbers;
        int rowBase = getRowsPerPage() * getCurrentPage();
        for (int row : newRowList)
            rowNumbers << QString::number(rowBase + row + 1); // +1 for visual representation of row, which in code is 0-based

        QMessageBox::StandardButton userResponse = QMessageBox::question(MAINWINDOW, tr("Delete rows"),
                              tr("You're about to delete newly inserted rows that are not committed yet. Row numbers: %1\n"
                                 "Such deletion will be permanent. Are you sure you want to delete them?")
                                    .arg(rowNumbers.join(", ")));

        if (userResponse == QMessageBox::Yes)
        {
            for (SqlQueryItem* item : newItemsToDelete)
                removeRow(item->index().row());
        }
    }


    emit commitStatusChanged(getUncommittedItems().size() > 0);
}

void SqlQueryModel::handlePossibleTableModification(Db *modDb, const QString &database, const QString &objName)
{
    QString dbName = database.toLower() == "main" ? QString() : database;
    DbAndTable dbAndTable(modDb, dbName, objName);
    if (tablesInUse.contains(dbAndTable))
        structureOutOfDate = true;
}

void SqlQueryModel::handlePossibleTableRename(Db *modDb, const QString &database, const QString &oldName, const QString &newName)
{
    UNUSED(newName);
    QString dbName = database.toLower() == "main" ? QString() : database;
    DbAndTable dbAndTable(modDb, dbName, oldName);
    if (tablesInUse.contains(dbAndTable))
        structureOutOfDate = true;
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

void SqlQueryModel::applyStrictFilter(const QString& value)
{
    UNUSED(value);
    // For custom query this is not supported.
}

void SqlQueryModel::applyRegExpFilter(const QString& value)
{
    UNUSED(value);
    // For custom query this is not supported.
}

void SqlQueryModel::applyStringFilter(const QStringList& values)
{
    UNUSED(values);
    // For custom query this is not supported.
}

void SqlQueryModel::applyStrictFilter(const QStringList& values)
{
    UNUSED(values);
    // For custom query this is not supported.
}

void SqlQueryModel::applyRegExpFilter(const QStringList& values)
{
    UNUSED(values);
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

    if (role == Qt::FontRole)
        return CFG_UI.Fonts.DataView.get();

    if (role == Qt::TextAlignmentRole && orientation == Qt::Horizontal)
        return Qt::AlignLeft;

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
    columns.clear();
    queryArgs.clear();
    conditions.clear();
    assignmentArgs.clear();
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
    this->columns = QStringList({column});
}

void SqlQueryModel::CommitUpdateQueryBuilder::addColumn(const QString& column)
{
    columns << column;
}

QString SqlQueryModel::CommitUpdateQueryBuilder::build()
{
    QString conditionsString = RowIdConditionBuilder::build();

    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database + ".";

    dbAndTable += table;

    int argIndex = 0;
    QString arg;
    QStringList assignments;
    for (QString& col : columns)
    {
        arg = ":value_" + QString::number(argIndex++);
        assignmentArgs << arg;
        assignments << col + " = " + arg;
    }

    return "UPDATE " + dbAndTable + " SET "+ assignments.join(", ") +" WHERE " + conditionsString + ";";
}

QStringList SqlQueryModel::CommitUpdateQueryBuilder::getAssignmentArgs() const
{
    return assignmentArgs;
}

void SqlQueryModel::SelectCellsQueryBuilder::addRowId(const RowId& rowId)
{
    if (includedRowIds.contains(rowId))
        return;

    static_qstring(argTempalate, ":rowIdArg%1");

    QStringList parts;
    QString arg;
    QHashIterator<QString,QVariant> it(rowId);
    while (it.hasNext())
    {
        it.next();
        arg = argTempalate.arg(argSquence++);
        queryArgs[arg] = it.value();
        parts << wrapObjIfNeeded(it.key()) + " = " + arg;
    }
    conditions << parts.join(" AND ");

    if (rowIdColumns.isEmpty())
    {
        rowIdColumns = toSet(rowId.keys());
        for (const QString& col : rowIdColumns)
            columns << wrapObjIfNeeded(col);
    }

    includedRowIds << rowId;
}

QString SqlQueryModel::SelectCellsQueryBuilder::build()
{
    QString conditionsString = conditions.join(" OR ");

    QString dbAndTable;
    if (!database.isNull())
        dbAndTable += database + ".";

    dbAndTable += table;

    QStringList selectColumns;
    for (const QString& col : columns)
    {
        // Explicit "ROWID" alias, because - if ROWID column
        // is the INTEGER PRIMARY KEY column - SQLite reports
        // column name as its table column name and it does not
        // match the RowId columns when requested in #readRowId().
        if (col.toUpper() == "ROWID")
            selectColumns << "ROWID AS ROWID";
        else
            selectColumns << col;
    }


    static_qstring(sql, "SELECT %1 FROM %2 WHERE %3;");
    return sql.arg(
                selectColumns.join(", "),
                dbAndTable,
                conditionsString
                );
}

void SqlQueryModel::SelectCellsQueryBuilder::clear()
{
    database = QString();
    table = QString();
    rowIdColumns.clear();
    columns.clear();
    conditions.clear();
    queryArgs.clear();
    includedRowIds.clear();
    argSquence = 0;
}

void SqlQueryModel::SelectCellsQueryBuilder::setDatabase(const QString& database)
{
    this->database = database;
}

void SqlQueryModel::SelectCellsQueryBuilder::setTable(const QString& table)
{
    this->table = table;
}

void SqlQueryModel::SelectCellsQueryBuilder::addColumn(const QString& column)
{
    this->columns << column;
}

RowId SqlQueryModel::SelectCellsQueryBuilder::readRowId(SqlResultsRowPtr row) const
{
    RowId rowId;
    for (const QString& column : rowIdColumns)
        rowId[column] = row->value(column);

    return rowId;
}

int SqlQueryModel::SelectCellsQueryBuilder::getColumnCount() const
{
    return columns.size();
}

QString SqlQueryModel::SelectCellsQueryBuilder::getTable() const
{
    return table;
}

QString SqlQueryModel::SelectCellsQueryBuilder::getDatabase() const
{
    return database;
}

bool SqlQueryModel::StoredFocus::isValid()
{
    return row > -1 && column > -1;
}

void SqlQueryModel::StoredFocus::reset()
{
    row = -1;
    column = -1;
    forFilter.clear();
    forRowsPerPage = -1;
    forPage = -1;
}
