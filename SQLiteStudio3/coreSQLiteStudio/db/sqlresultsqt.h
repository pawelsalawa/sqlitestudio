#ifndef SQLRESULTSQT_H
#define SQLRESULTSQT_H

#include "sqlresults.h"
#include "parser/ast/sqliteinsert.h"
#include <QSqlQuery>
#include <QStringList>

class Db;

/**
 * @brief The SqlResults implemented with Qt SQL classes
 */
class SqlResultsQt : public SqlResults
{
    public:
        /**
         * @brief Creates results object.
         */
        SqlResultsQt();

        /**
         * @brief Creates results basing on Qt's query object.
         * @param query Qt query object that was already executed.
         * @param db Database that the query was executed on.
         *
         * The query object must be already executed and this class just reads results from that object.
         * The ROWID is read from results using standart QtSql method - QSqlQuery.lastInsertId().
         */
        SqlResultsQt(QSqlQuery &query, Db* db);


        /**
         * @brief Creates results basing on Qt's query object.
         * @param query Qt query object that was already executed.
         * @param db Database that the query was executed on.
         * @param args Arguments passed to the query execution.
         *
         * This constructor calls the above constructor first, then it looks at the query string
         * and if it's INSERT, then it looks if the target table is WITHOUT ROWID table
         * and if it is, then it tries to get PRIMARY KEY parameters from args basing on
         * what were inserted values and what are primary key columns in the table.
         *
         * This will not work if the primary key column defines DEFAULT constraint,
         * because this constraint can be evaluated as an expression when the row is inserted,
         * so it's value cannot be predicted. In that case the full query reload is necessary
         * to enable edition capabilities for this row (user will be warned about it).
         */
        SqlResultsQt(QSqlQuery &query, Db* db, const QList<QVariant>& args);

        /**
         * @overload SqlResultsQt(QSqlQuery &query, Db* db, const QHash<QString,QVariant>& args)
         */
        SqlResultsQt(QSqlQuery &query, Db* db, const QHash<QString,QVariant>& args);

        /**
         * @brief Releases result resources.
         */
        ~SqlResultsQt();

        /**
         * @brief Defines RowId of recently INSERTed row.
         * @param rowId Row ID of the row.
         *
         * @warning Description of this method stands on how it should work, but because of SQLite limitations
         * the method is not currently used and therefore any effect caused by it is not visible to the application.
         * For more details on why is that see the note at the bottom of this description.
         *
         * This method is used for SQLite 3.8.2 or newer when the table is "WITHOUT ROWID". In that case after INSERT
         * the PK columns used for the insert are collected into the \p rowId and defined in the results.
         *
         * This is later retuned from SqlResults::getInsertRowId(). If this method is not called,
         * then result of SqlResults::getInsertRowId() is just a result of QSqlQuery::lastInsertId().
         *
         * All of this is used when SqlTableModel inserts new row - the new row ID is then provided from
         * QSqlQuery::lastInsertId() to let SQLiteStudio refer to that row.
         *
         * @note The "WITHOUT ROWID" tables edition capabilities are still limited, even SQLiteStudio supports it.
         * The limitation affects the rows that were INSERTed, but the whole table data view wasn't reloaded since then.
         * This is caused by impossibility of always getting PRIMARY KEY values after the row was inserted.
         * For example, if the PRIMARY KEY column has a DEFAULT constraint, where the default value is CURRENT_DATE,
         * then it's impossible to predict what was the value inserted into the row. We are also unable to read this single
         * row, because there's no clear way to query the row (as we don't know its key!). The only way is to reload
         * the whole table data. This way we have all rows with their keys. And so, this method is not used at the moment,
         * but might be used in future (once there is a good way to fix it), to handle the problem described here.
         */
        void setLastInsertRowId(const RowId& rowId);

        SqlResultsRowPtr next();
        bool hasNext();
        QString getErrorText();
        int getErrorCode();
        QStringList getColumnNames();
        int columnCount();
        qint64 rowCount();
        qint64 rowsAffected();
        RowId getInsertRowId();
        void restart();

    private:
        void readColumns();
        void handleInsert(const QList<QVariant>& args);
        void handleInsert(const QHash<QString,QVariant>& args);
        void extractInsert();
        void updateRowIdForInsert();
        RowId getRowId(const QString& query, const QList<QVariant>& args);
        RowId getRowId(const QString& query, const QHash<QString,QVariant>& args);

        QSqlQuery query;
        QStringList columns;
        SqliteInsertPtr insertStmt;
        QList<QVariant> insertArgList;
        QHash<QString,QVariant> insertArgHash;
        Db* db;
};

#endif // SQLRESULTSQT_H
