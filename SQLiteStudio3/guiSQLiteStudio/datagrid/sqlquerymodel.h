#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include "db/db.h"
#include "db/sqlquery.h"
#include "db/queryexecutor.h"
#include "sqlquerymodelcolumn.h"
#include "parser/ast/sqlitecreatetable.h"
#include "common/column.h"
#include "guiSQLiteStudio_global.h"
#include "sqlqueryitemdelegate.h"
#include "common/strhash.h"
#include <QStandardItemModel>
#include <QItemSelection>

class SqlQueryItem;
class FormView;
class SqlQueryView;
class SqlQueryRowNumModel;

class GUI_API_EXPORT SqlQueryModel : public QStandardItemModel
{
        Q_OBJECT

    public:
        enum Feature
        {
            INSERT_ROW = 0x01,
            DELETE_ROW = 0x02,
            FILTERING = 0x04
        };
        Q_DECLARE_FLAGS(Features, Feature)

        typedef std::function<void()> CommitSuccessfulHandler;

        friend class SqlQueryItemDelegate;

        explicit SqlQueryModel(QObject *parent = 0);
        virtual ~SqlQueryModel();

        static void staticInit();

        QString getQuery() const;
        void setQuery(const QString &value);
        void setExplainMode(bool explain);
        void setParams(const QHash<QString, QVariant>& params);
        Db* getDb() const;
        void setDb(Db* value);
        qint64 getExecutionTime();
        qint64 getTotalRowsReturned();
        qint64 getTotalRowsAffected();
        qint64 getTotalPages();
        QList<SqlQueryModelColumnPtr> getColumns();
        SqlQueryItem* itemFromIndex(const QModelIndex& index) const;
        SqlQueryItem* itemFromIndex(int row, int column) const;
        QModelIndexList findIndexes(int role, const QVariant &value, int hits = -1) const;
        QModelIndexList findIndexes(const QModelIndex &start, const QModelIndex& end, int role, const QVariant &value, int hits = -1, bool stringApproximation = false) const;
        QList<SqlQueryItem*> findItems(int role, const QVariant &value, int hits = -1) const;
        QList<SqlQueryItem*> findItems(const QModelIndex &start, const QModelIndex& end, int role, const QVariant &value, int hits = -1) const;
        SqlQueryItem* findAnyInColumn(int column, int role, const QVariant &value) const;
        QList<SqlQueryItem*> getUncommittedItems() const;
        QList<SqlQueryItem*> getRow(int row);
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        bool isExecutionInProgress() const;
        StrHash<QString> attachDependencyTables();
        void detachDependencyTables();

        /**
         * @brief Disables or re-enables async query execution
         * @param enabled True to set async mode enabled, false to set synchronous mode.
         *
         * This option is forwarded directly to the query executor.
         *
         * By default mode is asynchronous, but in some cases synchronous mode may be useful (like in FK combobox).
         */
        void setAsyncMode(bool enabled);
        virtual QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);
        virtual QString generateSelectFunctionQueryForItems(const QString& function, const QList<SqlQueryItem*>& items);
        virtual QString generateInsertQueryForItems(const QList<SqlQueryItem*>& items);
        virtual QString generateUpdateQueryForItems(const QList<SqlQueryItem*>& items);
        virtual QString generateDeleteQueryForItems(const QList<SqlQueryItem*>& items);

        virtual Features features() const;

        /**
         * @brief Request for applying SQL expression filtering on a dataset.
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as SQL expression to be placed after WHERE clause.
         */
        virtual void applySqlFilter(const QString& value);

        /**
         * @brief Request for applying an "equals" filtering on a dataset.
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as strict value to be compared against.
         */
        virtual void applyStrictFilter(const QString& value);

        /**
         * @brief Request for applying "LIKE" filtering on a dataset.
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as plain text to be matched in any column.
         */
        virtual void applyStringFilter(const QString& value);

        /**
         * @brief Request for applying Regular Expression filtering on a dataset.
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as regular expression to be matched in any column.
         */
        virtual void applyRegExpFilter(const QString& value);

        /**
         * @brief Request for applying "LIKE" filtering on a dataset.
         * @param values Filter expressions per column.
         * This is the same as applyStringFilter(const QString&), but is used for per-column filtering,
         * when user enters filtering expressions for each column sparately.
         */
        virtual void applyStringFilter(const QStringList& values);

        /**
         * @brief Request for applying an "equals" filtering on a dataset.
         * @param values Filter expressions per column.
         * This is the same as applyStrictFilter(const QString&), but is used for per-column filtering,
         * when user enters filtering expressions for each column sparately.
         */
        virtual void applyStrictFilter(const QStringList& values);

        /**
         * @brief Request for applying Regular Expression filtering on a dataset.
         * @param values Filter expressions per column.
         * This is the same as applyRegExpFilter(const QString&), but is used for per-column filtering,
         * when user enters filtering expressions for each column sparately.
         */
        virtual void applyRegExpFilter(const QStringList& values);

        /**
         * @brief resetFilter
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should resets filter, so the data is no longer filtered.
         */
        virtual void resetFilter();

        /**
         * @brief getCurrentPage Gets number of current results page
         * @param includeOneBeingLoaded If true, then also the page that is currently being loaded (but not yet done) will returned over the currently presented page.
         * @return Current page as 0-based index. If current page is not yet defined or paging is disabled, then this method returns 0.
         * This method returns always the page that is currently presented in results, not the one that might be currently being queried.
         * If you need to include the one being loaded (if any), then use getLoadingPage().
         */
        int getCurrentPage(bool includeOneBeingLoaded = false) const;
        void gotoPage(int newPage);
        bool canReload();
        virtual bool supportsModifyingQueriesInMenu() const;
        Qt::Alignment findValueAlignment(const QVariant& value, SqlQueryModelColumn* column);

        QueryExecutor::SortList getSortOrder() const;
        void setSortOrder(const QueryExecutor::SortList& newSortOrder);

        /**
         * @brief Tells if database schema was modified by last query executed.
         * @return true if schema was modified, or false if not.
         */
        bool wasSchemaModified() const;
        bool wasDataModifyingQuery() const;

        SqlQueryView* getView() const;
        void setView(SqlQueryView* value);

        static QList<QList<SqlQueryItem*>> groupItemsByRows(const QList<SqlQueryItem*>& items);
        static QHash<AliasedTable, QList<SqlQueryItem*> > groupItemsByTable(const QList<SqlQueryItem*>& items);
        static QHash<AliasedTable, QVector<SqlQueryModelColumn*> > groupColumnsByTable(const QVector<SqlQueryModelColumn*>& columns);

        bool getSimpleExecutionMode() const;
        void setSimpleExecutionMode(bool value);

        int getHardRowLimit() const;
        void setHardRowLimit(int value);

        bool isAllDataLoaded() const;

        bool isStructureOutOfDate() const;

        int getQueryCountLimitForSmartMode() const;
        void setQueryCountLimitForSmartMode(int value);

        void insertCustomRow(const QList<QVariant>& values, int insertionIndex);

        void setDesiredColumnWidth(int colIdx, int width);
        int getDesiredColumnWidth(int colIdx);

        void setCellDataLengthLimit(int value);
        int getCellDataLengthLimit();

    protected:
        class CommitUpdateQueryBuilder : public RowIdConditionBuilder
        {
            public:
                void clear();

                void setDatabase(const QString& database);
                void setTable(const QString& table);
                void setColumn(const QString& column);
                void addColumn(const QString& column);

                QString build();
                QStringList getAssignmentArgs() const;

            protected:
                QString database;
                QString table;
                QStringList columns;
                QStringList assignmentArgs;
        };

        class SelectCellsQueryBuilder : public RowIdConditionBuilder
        {
            public:
                void addRowId(const RowId& rowId);
                QString build();
                void clear();
                void setDatabase(const QString& database);
                void setTable(const QString& table);
                QString getDatabase() const;
                QString getTable() const;
                void addColumn(const QString& column);
                RowId readRowId(SqlResultsRowPtr row) const;
                int getColumnCount() const;

            protected:
                QSet<QString> rowIdColumns;
                QString database;
                QString table;
                QSet<QString> columns;
                QSet<RowId> includedRowIds;
                int argSquence = 0;
        };

        /**
         * @brief commitAddedRow Inserts new row to a table.
         * @param itemsInRow All cells for the new row.
         * @return true on success, false on failure.
         * Default implementation does nothing and returns false, because inserting for custom query results is not possible.
         * Inheriting class can reimplement this, so for example model specialized for single table can add rows.
         * The method implementation should take items that are in model (and are passed to this method)
         * and insert them into the actual database table. It also has to update items in the model,
         * so they are no longer "new" and have the same data as inserted into the database.
         */
        virtual bool commitAddedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);

        /**
         * @brief commitEditedRow Updates table row with new values.
         * @param itemsInRow Modified cell values.
         * @return true on success, false on failure.
         * Default implementation should be okay for most cases. It takes all modified cells and updates their
         * values in table basing on the ROWID, database, table and column names - which are all available,
         * unless the cell doesn't referr to the table, but in that case the cell should not be editable for user anyway.
         * <b>Important</b> thing to pay attention to is that the item list passed in arguments contains <b>only modified items</b>.
         */
        virtual bool commitEditedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);

        /**
         * @brief commitDeletedRow Deletes row from the table.
         * @param itemsInRow All cells for the deleted row.
         * @return true on success, false on failure.
         * Default implementation gets rid of row items from the model and that's all.
         * Inheriting class can reimplement this, so for example model specialized for single table can delete rows.
         * The method implementation should delete the row from the database.
         */
        virtual bool commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);

        /**
         * @brief rollbackAddedRow
         * @param itemsInRow All cells for the new row.
         * Default implementation gets rid of row items from the model and that's all.
         */
        virtual void rollbackAddedRow(const QList<SqlQueryItem*>& itemsInRow);

        /**
         * @brief rollbackEditedRow
         * @param itemsInRow All cells for the deleted row.
         * Restores original values in items.
         */
        virtual void rollbackEditedRow(const QList<SqlQueryItem*>& itemsInRow);

        /**
         * @brief rollbackDeletedRow
         * @param itemsInRow Modified cell values.
         * The implementation should restore original values to items in the model.
         * The default implementation is pretty much complete. It restores original state of row items.
         */
        virtual void rollbackDeletedRow(const QList<SqlQueryItem*>& itemsInRow);

        SqlQueryModelColumnPtr getColumnModel(const QString& database, const QString& table, const QString& column);
        SqlQueryModelColumnPtr getColumnModel(const QString& table, const QString& column);
        QList<SqlQueryModelColumnPtr> getTableColumnModels(const QString& database, const QString& table);
        QList<SqlQueryModelColumnPtr> getTableColumnModels(const QString& table);
        void updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId, SqlResultsRowPtr row,
                        const QStringList& columnNames, const BiStrHash& typeColumnToResColumn);
        void updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId);
        void updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId, Qt::Alignment alignment);
        RowId getNewRowId(const RowId& currentRowId, const QList<SqlQueryItem*> items);
        void updateRowIdForAllItems(const AliasedTable& table, const RowId& rowId, const RowId& newRowId);
        QHash<QString, QVariantList> toValuesGroupedByColumns(const QList<SqlQueryItem*>& items);
        QStringList toOrderedColumnNames(const QList<SqlQueryItem*>& items);
        void refreshGeneratedColumns(const QList<SqlQueryItem*>& items);
        void refreshGeneratedColumns(const QList<SqlQueryItem*>& items, QHash<SqlQueryItem*, QVariant>& values, const RowId& insertedRowId);

        QueryExecutor* queryExecutor = nullptr;
        Db* db = nullptr;
        QList<SqlQueryModelColumnPtr> columns;

        /**
         * @brief tablesInUse
         * List of tables that are used in currently presented data set.
         * Database in those tables (if not empty) is a symbolic name, as listed on database list.
         * If database is empty, then it was not explicitly typed in the query, so it's the local main db.
         */
        QList<DbAndTable> tablesInUse;

        /**
         * @brief Limit of data length in loaded cells.
         *
         * Bytes or utf-8 characters.
         * Having this set to 10000 gives about 290 MB of memory consumption
         * while having 30 columns and 1000 result rows loaded, all with 10000 bytes.
         */
        int cellDataLengthLimit = 100;

    private:
        struct TableDetails
        {
            struct ColumnDetails
            {
                SqliteColumnTypePtr type;
                QList<SqliteCreateTable::Column::ConstraintPtr> constraints;
            };

            QHash<QString,ColumnDetails> columns;
            QList<SqliteCreateTable::ConstraintPtr> constraints;
        };

        /**
         * @brief Loads data from queyr execution into UI cells.
         * @param results Execution results from query executor.
         * @return Whether to continue execution or not.
         */
        bool loadData(SqlQueryPtr results);

        QList<QStandardItem*> loadRow(SqlResultsRowPtr row, SqlQueryPtr results);
        RowId getRowIdValue(SqlResultsRowPtr row, int columnIdx);
        bool readColumns();
        void readColumnDetails();
        void updateColumnsHeader();
        void updateColumnHeaderLabels();
        void executeQueryInternal();
        void internalExecutionStopped();
        QHash<AliasedTable,TableDetails> readTableDetails();
        QList<AliasedTable> getTablesForColumns();
        QList<bool> getColumnEditionEnabledList();
        QList<SqlQueryItem*> toItemList(const QModelIndexList& indexes) const;
        bool commitRow(const QList<SqlQueryItem*>& itemsInRow, QList<CommitSuccessfulHandler>& successfulCommitHandlers);
        void rollbackRow(const QList<SqlQueryItem*>& itemsInRow);
        QHash<SqlQueryItem*, QVariant> readCellValues(SelectCellsQueryBuilder& queryBuilder, const QHash<RowId, QSet<SqlQueryItem*> >& itemsPerRowId);
        void storeStep1NumbersFromExecution();
        void storeStep2NumbersFromExecution();
        void restoreNumbersToQueryExecutor();
        QList<SqlQueryItem*> filterOutCommittedItems(const QList<SqlQueryItem*>& items);
        void commitInternal(const QList<SqlQueryItem*>& items);
        void rollbackInternal(const QList<SqlQueryItem*>& items);
        void reloadInternal();
        void addNewRowInternal(int rowIdx);
        Icon& getIconForIdx(int idx) const;
        void detachDatabases();
        QString getDatabaseForCommit(const QString& database);
        void recalculateRowsAndPages(int rowsDelta);
        int getInsertRowIndex();
        void notifyItemEditionEnded(const QModelIndex& idx);
        int getRowsPerPage() const;
        bool isEmptyQuery() const;

        QString query;
        QHash<QString, QVariant> queryParams;
        bool explain = false;
        bool simpleExecutionMode = false;

        /**
         * @brief reloadAvailable
         * This value is false by default and is changed only once - after first successful
         * query execution. It's designed to report proper status by canReload().
         * Data reloading is available to user practically after any query was executed.
         */
        bool reloadAvailable = false;

        /**
         * @brief reloading
         * This switch tells you if model is in the process of data reloading (true value)
         * or initial query execution (then it's false). Data reloading takes place in any case
         * when page is changed, order is changed, or simply user calls the data reloading.
         * The initial query execution takes place when user calls "Execute query",
         * which is translated to call to executeQuery().
         */
        bool reloading = false;

        /**
         * @brief lastExecutionTime
         * Keeps number of milliseconds that recently successfully executed query took to complete.
         * If there was no such query executed, this will be 0.
         */
        quint64 lastExecutionTime = 0;

        /**
         * @brief totalRowsReturned
         * Keeps number of rows returned from recently successfully executed query.
         * If there was no such query executed, this will be 0.
         */
        quint64 totalRowsReturned = 0;

        /**
         * @brief rowsAffected
         * Keeps number of rows affected by recently successfully executed query.
         * If there was no such query executed, this will be 0.
         */
        quint64 rowsAffected = 0;

        /**
         * @brief totalPages
         * Keeps number of pages available in recently successfully executed query.
         * If there was no such query executed, this will be -1.
         */
        int totalPages = -1;

        /**
         * @brief page
         * The page variable keeps page of recently sucessfly loaded data.
         * If there was no successful data load, or when paging is disabled, then this will be -1.
         */
        int page = -1;

        /**
         * @brief sortOrder
         * The sortOrder variable keeps sorting order of recently sucessfly loaded data.
         * If column member of the sort object is -1, then no sorting is being aplied.
         */
        QueryExecutor::SortList sortOrder;

        QHash<Column,SqlQueryModelColumnPtr> columnMap;
        QHash<AliasedColumn,int> columnWidths;
        QHash<AliasedTable,QHash<QString,QString>> tableToRowIdColumn;
        QStringList headerColumns;
        int rowNumBase = 0;
        SqlQueryView* view = nullptr;
        quint32 resultsCountingAsyncId = 0;
        QStringList requiredDbAttaches;
        StrHash<QString> dbNameToAttachNameMapForCommit;
        QList<Db*> dbListToDetach;

        /**
         * @brief Sets row count limit, despite user configured limit.
         *
         * -1 to not apply hard limit (use user configured row limit), any other value is the limit.
         */
        int hardRowLimit = -1;

        /**
         * @brief Limit for rows in case there is many columns.
         *
         * -1 to not apply the limit. This is set during reading columns. If there is many columns,
         * we need to keep maximum limit of rows at pace, so we don't overuse the RAM.
         * This limit is soft, meaning it applies only if it's smaller than configured limit or hardRowLimit.
         * If any of two limits mentioned above are smaller, this limit will not come to the play.
         */
        int columnRatioBasedRowLimit = -1;

        int resultColumnCount = 0;

        /**
         * @brief tablesForColumns
         * List of tables associated to \link #columns by order index.
         */
        QList<AliasedTable> tablesForColumns;

        /**
         * @brief columnEditionStatus
         * List of column edition capabilities, in the same order as \link #columns.
         */
        QList<bool> columnEditionStatus;

        QList<int> rowsDeletedSuccessfullyInTheCommit;

        bool allDataLoaded = false;

        bool structureOutOfDate = false;

        /**
         * @brief Set of existing model objects, updated for each construction and destruction.
         *
         * This is used by loadData() to determinathe whether processEvents() caused deletion of the model.
         * We need to keep processEvents() call so the UI is responsive to Interrupt button,
         * but this causes crash when model is deleted in the events processing (like when FK combobox is deleted faster than data is loaded).
         */
        static QSet<SqlQueryModel*> existingModels;

    private slots:
        void handleExecFinished(SqlQueryPtr results);
        void handleExecFailed(int code, QString errorMessage);
        void resultsCountingFinished(quint64 rowsAffected, quint64 rowsReturned, int totalPages);

    public slots:
        void itemValueEdited(SqlQueryItem* item);
        void repaintAllItems();
        void changeSorting(int logicalIndex, Qt::SortOrder order);
        void changeSorting(int logicalIndex);
        void firstPage();
        void prevPage();
        void nextPage();
        void lastPage();
        void executeQuery();
        void interrupt();
        void commit();
        void rollback();
        void commit(const QList<SqlQueryItem*>& items);
        void rollback(const QList<SqlQueryItem*>& items);
        void reload();
        void updateSelectiveCommitRollbackActions(const QItemSelection& selected, const QItemSelection& deselected);
        void addNewRow();
        void addMultipleRows();
        void deleteSelectedRows();
        void handlePossibleTableModification(Db* modDb, const QString& database, const QString& objName);
        void handlePossibleTableRename(Db* modDb, const QString& database, const QString& oldName, const QString& newName);

    signals:
        /**
         * @brief executionStarted
         *
         * Emitted just after query started executing.
         */
        void executionStarted();

        /**
         * @brief executionSuccessful
         *
         * Emitted after initial query execution was successful. It's not emitted after data reloading of page changing.
         */
        void executionSuccessful();

        /**
         * @brief Execution is finished and data is about to be loaded to model.
         * Emitted every query execution, every data reloading and every page change.
         */
        void aboutToLoadResults();

        /**
         * @brief executionFailed
         * @param errorText
         *
         * Emitted after failed query execution, or data reloading failed or page changing failed.
         */
        void executionFailed(const QString& errorText);

        /**
         * @brief loadingEnded
         * @param executionSuccessful
         *
         * Emitted every query execution, every data reloading and every page change.
         */
        void loadingEnded(bool executionSuccessful);

        /**
         * @brief totalRowsAndPagesAvailable
         *
         * Emitted when model finished querying total number of rows (and pages).
         * This is asynchronously emitted after execution has finished, so counting doesn't block the model.
         * It might not get emitted in some cases, like when there was an error when counting (it will be logged with qWarning()),
         * or when counting was interrupted by executing query (the same, or modified).
         *
         * When the main query execution failed, this signal will be emitted to inform about total rows and pages being 0.
         */
        void totalRowsAndPagesAvailable();

        void storeExecutionInHistory();

        /**
         * @brief commitStatusChanged
         * @param commitAvailable Tells if there's anything to commit/rollback or not.
         *
         * Emitted after any results cell has been modified and can now be committed or rolled back.
         * Also emitted after commit and rollback.
         */
        void commitStatusChanged(bool commitAvailable);

        /**
         * @brief selectiveCommitStatusChanged
         * @param commitAvailable Tells if there's anything to commit/rollback or not.
         *
         * Emitted when user changes selection in the view, so if the selection includes any uncommitted cells,
         * then this signal will be emitted with parameter true, or if there is no uncommitted cells,
         * then it will be emitted with parameter false.
         */
        void selectiveCommitStatusChanged(bool commitAvailable);

        /**
         * @brief sortIndicatorUpdated
         *
         * Emitted after columns header sorting has been changed.
         */
        void sortingUpdated(const QueryExecutor::SortList& sortOrder);

        void aboutToCommit(int totalSteps);
        void committingStepFinished(int step);
        void commitFinished();
        void itemEditionEnded(SqlQueryItem* item);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SqlQueryModel::Features)

#endif // SQLQUERYMODEL_H
