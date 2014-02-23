#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include "db/db.h"
#include "db/queryexecutor.h"
#include "sqlquerymodelcolumn.h"
#include "parser/ast/sqlitecreatetable.h"
#include "column.h"
#include <QStandardItemModel>
#include <QItemSelection>

class SqlQueryItem;
class FormView;
class SqlQueryView;
class SqlQueryRowNumModel;

class SqlQueryModel : public QStandardItemModel
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

        explicit SqlQueryModel(QObject *parent = 0);
        virtual ~SqlQueryModel();

        static void staticInit();

        QString getQuery() const;
        void setQuery(const QString &value);
        void setExplainMode(bool explain);
        Db* getDb() const;
        void setDb(Db* value);
        qint64 getExecutionTime();
        qint64 getTotalRowsReturned();
        qint64 getTotalRowsAffected();
        qint64 getTotalPages();
        QList<SqlQueryModelColumnPtr> getColumns();
        SqlQueryItem* itemFromIndex(const QModelIndex& index) const;
        SqlQueryItem* itemFromIndex(int row, int column) const;
        static int getCellDataLengthLimit();
        QModelIndexList findIndexes(int role, const QVariant &value, int hits = -1) const;
        QModelIndexList findIndexes(const QModelIndex &start, const QModelIndex& end, int role, const QVariant &value, int hits = -1) const;
        QList<SqlQueryItem*> findItems(int role, const QVariant &value, int hits = -1) const;
        QList<SqlQueryItem*> findItems(const QModelIndex &start, const QModelIndex& end, int role, const QVariant &value, int hits = -1) const;
        QList<SqlQueryItem*> getUncommitedItems() const;
        QList<SqlQueryItem*> getRow(int row);
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

        virtual Features features() const;

        /**
         * @brief applySqlFilter
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as SQL expression to be placed after WHERE clause.
         */
        virtual void applySqlFilter(const QString& value);

        /**
         * @brief applyStringFilter
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as plain text to be matched in any column.
         */
        virtual void applyStringFilter(const QString& value);

        /**
         * @brief applyStringFilter
         * @param value Filter expression.
         * Default implementation does nothing. Working implementation (i.e. for a table)
         * should set the query to temporary value which respects given filter and reload the data.
         * Filter passed to this method is meant to be treated as regular expression to be matched in any column.
         */
        virtual void applyRegExpFilter(const QString& value);

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

        QueryExecutor::Sort getSortOrder() const;

        static QList<QList<SqlQueryItem*> > groupItemsByRows(const QList<SqlQueryItem*>& items);

        SqlQueryView* getView() const;
        void setView(SqlQueryView* value);

    protected:
        class CommitUpdateQueryBuilder
        {
            public:
                void clear();

                void setDatabase(const QString& database);
                void setTable(const QString& table);
                void setColumn(const QString& column);
                void setRowId(const RowId& rowId);

                QString build();
                const QHash<QString,QVariant>& getQueryArgs();

            protected:
                QString database;
                QString table;
                QString column;
                QStringList conditions;
                QHash<QString,QVariant> queryArgs;
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
        virtual bool commitAddedRow(const QList<SqlQueryItem*>& itemsInRow);

        /**
         * @brief commitEditedRow Updates table row with new values.
         * @param itemsInRow Modified cell values.
         * @return true on success, false on failure.
         * Default implementation should be okay for most cases. It takes all modified cells and updates their
         * values in table basing on the ROWID, database, table and column names - which are all available,
         * unless the cell doesn't referr to the table, but in that case the cell should not be editable for user anyway.
         * <b>Important</b> thing to pay attention to is that the item list passed in arguments contains <b>only modified items</b>.
         */
        virtual bool commitEditedRow(const QList<SqlQueryItem*>& itemsInRow);

        /**
         * @brief commitDeletedRow Deletes row from the table.
         * @param itemsInRow All cells for the deleted row.
         * @return true on success, false on failure.
         * Default implementation gets rid of row items from the model and that's all.
         * Inheriting class can reimplement this, so for example model specialized for single table can delete rows.
         * The method implementation should delete the row from the database.
         */
        virtual bool commitDeletedRow(const QList<SqlQueryItem*>& itemsInRow);

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
        void updateItem(SqlQueryItem* item, const QVariant& value, int columnIndex, const RowId& rowId);

        QueryExecutor* queryExecutor;
        Db* db = nullptr;
        QList<SqlQueryModelColumnPtr> columns;

        /**
         * @brief cellDataLengthLimit
         * Bytes or utf-8 characters.
         * Having this set to 10000 gives about 290 MB of memory consumption
         * while having 30 columns and 1000 result rows loaded, all with 10000 bytes.
         */
        static const int cellDataLengthLimit = 10000;

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

        void loadData(SqlResultsPtr results);
        QList<QStandardItem*> loadRow(SqlResultsRowPtr row);
        RowId getRowIdValue(SqlResultsRowPtr row, int columnIdx);
        void readColumns();
        void readColumnDetails();
        void updateColumnsHeader();
        void updateColumnHeaderLabels();
        void executeQueryInternal();
        QHash<Table,TableDetails> readTableDetails();
        QList<Table> getTablesForColumns();
        QList<bool> getColumnEditionEnabledList();
        QList<SqlQueryItem*> toItemList(const QModelIndexList& indexes) const;
        bool commitRow(const QList<SqlQueryItem*>& itemsInRow);
        void rollbackRow(const QList<SqlQueryItem*>& itemsInRow);
        void storeStep1NumbersFromExecution();
        void storeStep2NumbersFromExecution();
        void restoreNumbersToQueryExecutor();
        QList<SqlQueryItem*> filterOutCommitedItems(const QList<SqlQueryItem*>& items);
        void commitInternal(const QList<SqlQueryItem*>& items);
        void rollbackInternal(const QList<SqlQueryItem*>& items);
        void reloadInternal();
        void addNewRowInternal(int rowIdx);

        QString query;
        bool explain = false;

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
         * Keeps number of milliseconds that recently successfly executed query took to complete.
         * If there was no such query executed, this will be 0.
         */
        quint64 lastExecutionTime = 0;

        /**
         * @brief totalRowsReturned
         * Keeps number of rows returned from recently successfly executed query.
         * If there was no such query executed, this will be 0.
         */
        quint64 totalRowsReturned = 0;

        /**
         * @brief rowsAffected
         * Keeps number of rows affected by recently successfly executed query.
         * If there was no such query executed, this will be 0.
         */
        quint64 rowsAffected = 0;

        /**
         * @brief totalPages
         * Keeps number of pages available in recently successfly executed query.
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
        QueryExecutor::Sort sortOrder;

        QHash<Column,SqlQueryModelColumnPtr> columnMap;
        QHash<Table,QHash<QString,QString>> tableToRowIdColumn;
        QStringList headerColumns;
        int rowNumBase = 0;
        SqlQueryView* view = nullptr;
        quint32 resultsCountingAsyncId = 0;

        /**
         * @brief rowIdColumns
         * We skip first this number of columns from the results of the SQL query, because those are ROWID columns.
         * The query returns ROWID columns, because this is how QueryExecutor provides this information.
         */
        int rowIdColumns = 0;

        /**
         * @brief tablesForColumns
         * List of tables associated to \link #columns by order index.
         */
        QList<Table> tablesForColumns;

        /**
         * @brief columnEditionStatus
         * List of column edition capabilities, in the same order as \link #columns.
         */
        QList<bool> columnEditionStatus;

    private slots:
        void handleExecFinished(SqlResultsPtr results);
        void handleExecFailed(int code, QString errorMessage);
        void resultsCountingFinished(quint64 rowsAffected, quint64 rowsReturned, int totalPages);

    public slots:
        void itemValueEdited(SqlQueryItem* item);
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

    signals:
        /**
         * @brief executionStarted
         * Emitted just after query started executing.
         */
        void executionStarted();

        /**
         * @brief executionSuccessful
         * Emitted after initial query execution was successful. It's not emitted after data reloading of page changing.
         */
        void executionSuccessful();

        /**
         * @brief executionFailed
         * @param errorText
         * Emitted after failed query execution, or data reloading failed or page changing failed.
         */
        void executionFailed(const QString& errorText);

        /**
         * @brief loadingEnded
         * @param executionSuccessful
         * Emitted every query execution, every data reloading and every page change.
         */
        void loadingEnded(bool executionSuccessful);

        /**
         * @brief totalRowsAndPagesAvailable
         * Emitted when model finished querying total number of rows (and pages).
         * This is asynchronously emitted after execution has finished, so counting doesn't block the model.
         * It might not get emitted in some cases, like when there was an error when counting (it will be logged with qWarning()),
         * or when counting was interrupted by executing query (the same, or modified).
         *
         * When the main query execution failed, this signal will be emitted to inform about total rows and pages being 0.
         */
        void totalRowsAndPagesAvailable();

        /**
         * @brief commitStatusChanged
         * @param commitAvailable Tells if there's anything to commit/rollback or not.
         * Emitted after any results cell has been modified and can now be commited or rolled back.
         * Also emitted after commit and rollback.
         */
        void commitStatusChanged(bool commitAvailable);

        /**
         * @brief selectiveCommitStatusChanged
         * @param commitAvailable Tells if there's anything to commit/rollback or not.
         * Emitted when user changes selection in the view, so if the selection includes any uncommited cells,
         * then this signal will be emitted with parameter true, or if there is no uncommited cells,
         * then it will be emitted with parameter false.
         */
        void selectiveCommitStatusChanged(bool commitAvailable);

        /**
         * @brief sortIndicatorUpdated
         * @param logicalIndex
         * @param order
         * Emitted after columns header sorting has been changed.
         */
        void sortingUpdated(int logicalIndex, Qt::SortOrder order);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SqlQueryModel::Features)

#endif // SQLQUERYMODEL_H
