#ifndef ABSTRACTDB2_H
#define ABSTRACTDB2_H

#include "db/abstractdb.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include "common/unused.h"
#include "db/sqlerrorcodes.h"
#include "db/sqlerrorresults.h"
#include <sqlite.h>
#include <QThread>
#include <QPointer>
#include <QDebug>

/**
 * @brief Complete implementation of SQLite 2 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 2. In most cases you will only need
 * to create one public constructor, which forwards parameters to the AbstractDb constructor.
 * This be sufficient to implement SQLite 2 database plugin.
 * Just link it with proper SQLite library.
 *
 * The template parameter is currently not used for anything specific, so pass any unique type name.
 * The best would be to define empty class/structure just for this purpose.
 * The parameter is there, so this class becomes a template class.
 * We need a template class so we can provide common code base for all SQLite 2 plugins, while the
 * code doesn't introduce dependency to SQLite 2 library, until it's used, which is in SQLite 2 plugins.
 *
 * @see DbQt
 */
template <class T>
class AbstractDb2 : public AbstractDb
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb constructor.
         */
        AbstractDb2(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        ~AbstractDb2();

    protected:
        bool isOpenInternal();
        void interruptExecution();
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args);
        SqlResultsPtr execInternal(const QString& query, const QHash<QString, QVariant>& args);
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        void initAfterOpen();
        QString getTypeLabel();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount);
        bool registerAggregateFunction(const QString& name, int argCount);
        bool registerCollationInternal(const QString& name);
        bool deregisterCollationInternal(const QString& name);

    private:
        class Results : public SqlResults
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        void init(const QStringList& columns, const QList<QVariant>& resultValues);
                };

                Results(AbstractDb2<T>* db, sqlite_vm* stmt, bool error);
                ~Results();

                QString getErrorText();
                int getErrorCode();
                QStringList getColumnNames();
                int columnCount();
                qint64 rowsAffected();

            protected:
                SqlResultsRowPtr nextInternal();
                bool hasNextInternal();

            private:
                int fetchNext();
                void init(int columnsCount, const char** columns);

                QPointer<AbstractDb2<T>> db;
                sqlite_vm* stmt = nullptr;
                int errorCode = SQLITE_OK;
                QString errorMessage;
                int colCount = -1;
                QStringList colNames;
                QList<QVariant> nextRowValues;
                int affected = 0;
                bool rowAvailable = true;
        };

        int prepareStmt(const QString& query, sqlite_vm** stmt);
        int bindParam(sqlite_vm* stmt, int paramIdx, const QVariant& value);
        bool isValid(sqlite_vm* stmt) const;
        void cleanUp();
        QString freeStatement(sqlite_vm* stmt);

        static void storeResult(sqlite_func* func, const QVariant& result, bool ok);
        static QList<QVariant> getArgs(int argCount, const char** args);
        static void evaluateScalar(sqlite_func* func, int argCount, const char** args);
        static void evaluateAggregateStep(sqlite_func* func, int argCount, const char** args);
        static void evaluateAggregateFinal(sqlite_func* func);
        static void* getContextMemPtr(sqlite_func* func);
        static QHash<QString,QVariant> getAggregateContext(sqlite_func* func);
        static void setAggregateContext(sqlite_func* func, const QHash<QString,QVariant>& aggregateContext);
        static void releaseAggregateContext(sqlite_func* func);
        static QString replaceNamedParams(const QString& query);

        sqlite* dbHandle = nullptr;
        QString lastError;
        int lastErrorCode = SQLITE_OK;
        QList<sqlite_vm*> preparedStatements;
        QList<FunctionUserData*> userDataList;
};

//------------------------------------------------------------------------------------
// AbstractDb2
//------------------------------------------------------------------------------------

template <class T>
AbstractDb2<T>::AbstractDb2(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb(name, path, connOptions)
{
}

template <class T>
AbstractDb2<T>::~AbstractDb2()
{
    if (isOpenInternal())
        closeInternal();
}

template <class T>
bool AbstractDb2<T>::isOpenInternal()
{
    return dbHandle != nullptr;
}

template <class T>
void AbstractDb2<T>::interruptExecution()
{
    if (!isOpenInternal())
        return;

    sqlite_interrupt(dbHandle);
}

template <class T>
SqlResultsPtr AbstractDb2<T>::execInternal(const QString& query, const QList<QVariant>& args)
{
    sqlite_vm* stmt;
    int res;
    int paramIdx;

    QList<QueryWithParamCount> queries = getQueriesWithParamCount(query, Dialect::Sqlite2);

    foreach (const QueryWithParamCount singleQuery, queries)
    {
        res = prepareStmt(singleQuery.first, &stmt);
        if (res != SQLITE_OK)
            return SqlResultsPtr(new Results(this, nullptr, true));

        for (paramIdx = 1; paramIdx <= singleQuery.second; paramIdx++)
        {
            res = bindParam(stmt, paramIdx, args[paramIdx-1]);
            if (res != SQLITE_OK)
                return SqlResultsPtr(new Results(this, stmt, true));
        }
    }
    return SqlResultsPtr(new Results(this, stmt, false));
}

template <class T>
SqlResultsPtr AbstractDb2<T>::execInternal(const QString& query, const QHash<QString, QVariant>& args)
{
    sqlite_vm* stmt;
    int res;
    int paramIdx;

    QList<QueryWithParamNames> queries = getQueriesWithParamNames(query, Dialect::Sqlite2);

    QString singleQueryStr;
    foreach (const QueryWithParamNames singleQuery, queries)
    {
        singleQueryStr = replaceNamedParams(singleQuery.first);
        res = prepareStmt(singleQueryStr, &stmt);
        if (res != SQLITE_OK)
            return SqlResultsPtr(new Results(this, nullptr, true));

        paramIdx = 1;
        foreach (const QString& paramName, singleQuery.second)
        {
            if (!args.contains(paramName))
            {
                lastErrorCode = SqlErrorCode::OTHER_EXECUTION_ERROR;
                lastError = "Error while preparing statement: could not bind parameter " + paramName;
                return SqlResultsPtr(new Results(this, stmt, true));
            }

            res = bindParam(stmt, paramIdx++, args[paramName]);
            if (res != SQLITE_OK)
                return SqlResultsPtr(new Results(this, stmt, true));
        }
    }

    return SqlResultsPtr(new Results(this, stmt, false));
}

template <class T>
QString AbstractDb2<T>::replaceNamedParams(const QString& query)
{
    TokenList tokens = Lexer::tokenize(query, Dialect::Sqlite2);
    for (TokenPtr token : tokens)
    {
        if (token->type == Token::BIND_PARAM)
            token->value = "?";
    }
    return tokens.detokenize();
}

template <class T>
QString AbstractDb2<T>::getErrorTextInternal()
{
    return lastError;
}

template <class T>
int AbstractDb2<T>::getErrorCodeInternal()
{
    return lastErrorCode;
}

template <class T>
bool AbstractDb2<T>::openInternal()
{
    sqlite* handle;
    char* errMsg = nullptr;
    handle = sqlite_open(path.toUtf8().constData(), 0, &errMsg);
    if (!handle)
    {
        lastErrorCode = SQLITE_ERROR;

        if (errMsg)
        {
            lastError = tr("Could not open database: %1").arg(QString::fromUtf8(errMsg));
            sqlite_freemem(errMsg);
        }
        return false;
    }
    dbHandle = handle;
    return true;
}

template <class T>
bool AbstractDb2<T>::closeInternal()
{
    if (!dbHandle)
        return false;

    cleanUp();

    sqlite_close(dbHandle);
    dbHandle = nullptr;
    return true;
}

template <class T>
void AbstractDb2<T>::initAfterOpen()
{
}

template <class T>
QString AbstractDb2<T>::getTypeLabel()
{
    return T::label;
}

template <class T>
bool AbstractDb2<T>::deregisterFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    sqlite_create_function(dbHandle, name.toLatin1().data(), argCount, nullptr, nullptr);
    sqlite_create_aggregate(dbHandle, name.toLatin1().data(), argCount, nullptr, nullptr, nullptr);

    FunctionUserData* userData = nullptr;
    QMutableListIterator<FunctionUserData*> it(userDataList);
    while (it.hasNext())
    {
        userData = it.next();
        if (userData->name == name && userData->argCount == argCount)
        {
            it.remove();
            delete userData;
        }
    }

    return true;
}

template <class T>
bool AbstractDb2<T>::registerScalarFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;
    userDataList << userData;

    int res = sqlite_create_function(dbHandle, name.toUtf8().constData(), argCount,
                                     &AbstractDb2<T>::evaluateScalar, userData);

    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb2<T>::registerAggregateFunction(const QString& name, int argCount)
{
    if (!dbHandle)
        return false;

    FunctionUserData* userData = new FunctionUserData;
    userData->db = this;
    userData->name = name;
    userData->argCount = argCount;
    userDataList << userData;

    int res = sqlite_create_aggregate(dbHandle, name.toUtf8().constData(), argCount,
                                      &AbstractDb2<T>::evaluateAggregateStep,
                                      &AbstractDb2<T>::evaluateAggregateFinal,
                                      userData);

    return res == SQLITE_OK;
}

template <class T>
bool AbstractDb2<T>::registerCollationInternal(const QString& name)
{
    // Not supported in SQLite 2
    UNUSED(name);
    return false;
}

template <class T>
bool AbstractDb2<T>::deregisterCollationInternal(const QString& name)
{
    // Not supported in SQLite 2
    UNUSED(name);
    return false;
}

template <class T>
int AbstractDb2<T>::prepareStmt(const QString& query, sqlite_vm** stmt)
{
    char* errMsg = nullptr;
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    int res = sqlite_compile(dbHandle, queryBytes.constData(), &tail, stmt, &errMsg);
    if (res != SQLITE_OK)
    {
        if (errMsg)
        {
            lastErrorCode = res;
            lastError = QString::fromUtf8((errMsg));
            sqlite_freemem(errMsg);
        }
        return res;
    }

    preparedStatements << *stmt;

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return SQLITE_OK;
}

template <class T>
int AbstractDb2<T>::bindParam(sqlite_vm* stmt, int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return sqlite_bind(stmt, paramIdx, nullptr, 0, 0);
    }

    switch (value.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            return sqlite_bind(stmt, paramIdx, ba.constData(), ba.size(), true);
        }
        default:
        {
            QByteArray ba = value.toString().toLatin1();
            ba.append('\0');
            return sqlite_bind(stmt, paramIdx, ba.constData(), ba.size(), true);
        }
    }

    return SQLITE_MISUSE; // not going to happen
}

template <class T>
bool AbstractDb2<T>::isValid(sqlite_vm* stmt) const
{
    return preparedStatements.contains(stmt);
}

template <class T>
void AbstractDb2<T>::cleanUp()
{
    char* errMsg;
    foreach (sqlite_vm* stmt, preparedStatements)
    {
        errMsg = nullptr;
        sqlite_finalize(stmt, &errMsg);
        if (errMsg)
        {
            qWarning() << "Error while call to sqlite_finalize():" << errMsg;
            sqlite_freemem(errMsg);
        }
    }

    preparedStatements.clear();
}

template <class T>
QString AbstractDb2<T>::freeStatement(sqlite_vm* stmt)
{
    if (!isValid(stmt))
        return QString::null;

    char* errMsg = nullptr;
    sqlite_finalize(stmt, &errMsg);

    QString message;
    if (errMsg)
    {
        message = QString::fromUtf8(errMsg);
        sqlite_freemem(errMsg);
    }

    preparedStatements.removeOne(stmt);
    return message;
}

template <class T>
void AbstractDb2<T>::storeResult(sqlite_func* func, const QVariant& result, bool ok)
{
    if (!ok)
    {
        QByteArray ba = result.toString().toUtf8();
        sqlite_set_result_error(func, ba.constData(), ba.size());
        return;
    }

    // Code below is a modified code from Qt (its SQLite plugin).
    if (result.isNull())
    {
        sqlite_set_result_string(func, nullptr, -1);
        return;
    }

    switch (result.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = result.toByteArray();
            sqlite_set_result_string(func, ba.constData(), ba.size());
            break;
        }
        case QVariant::Int:
        case QVariant::Bool:
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            sqlite_set_result_int(func, result.toInt());
            break;
        }
        case QVariant::Double:
        {
            sqlite_set_result_double(func, result.toDouble());
            break;
        }
        default:
        {
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            QByteArray ba = result.toString().toUtf8();
            sqlite_set_result_string(func, ba.constData(), ba.size());
            break;
        }
    }
}

template <class T>
QList<QVariant> AbstractDb2<T>::getArgs(int argCount, const char** args)
{
    QList<QVariant> results;

    for (int i = 0; i < argCount; i++)
    {
        if (!args[i])
        {
            results << QVariant();
            continue;
        }

        results << QString::fromUtf8(args[i]);
    }
    return results;
}

template <class T>
void AbstractDb2<T>::evaluateScalar(sqlite_func* func, int argCount, const char** args)
{
    QList<QVariant> argList = getArgs(argCount, args);
    bool ok = true;
    QVariant result = AbstractDb::evaluateScalar(sqlite_user_data(func), argList, ok);
    storeResult(func, result, ok);
}

template <class T>
void AbstractDb2<T>::evaluateAggregateStep(sqlite_func* func, int argCount, const char** args)
{
    void* dataPtr = sqlite_user_data(func);
    QList<QVariant> argList = getArgs(argCount, args);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(func);

    AbstractDb::evaluateAggregateStep(dataPtr, aggregateContext, argList);

    setAggregateContext(func, aggregateContext);
}

template <class T>
void AbstractDb2<T>::evaluateAggregateFinal(sqlite_func* func)
{
    void* dataPtr = sqlite_user_data(func);
    QHash<QString,QVariant> aggregateContext = getAggregateContext(func);

    bool ok = true;
    QVariant result = AbstractDb::evaluateAggregateFinal(dataPtr, aggregateContext, ok);

    storeResult(func, result, ok);
    releaseAggregateContext(func);
}

template <class T>
void*AbstractDb2<T>::getContextMemPtr(sqlite_func* func)
{
    return sqlite_aggregate_context(func, sizeof(QHash<QString,QVariant>**));
}

template <class T>
QHash<QString, QVariant> AbstractDb2<T>::getAggregateContext(sqlite_func* func)
{
    return AbstractDb::getAggregateContext(getContextMemPtr(func));
}

template <class T>
void AbstractDb2<T>::setAggregateContext(sqlite_func* func, const QHash<QString, QVariant>& aggregateContext)
{
    AbstractDb::setAggregateContext(getContextMemPtr(func), aggregateContext);
}

template <class T>
void AbstractDb2<T>::releaseAggregateContext(sqlite_func* func)
{
    AbstractDb::releaseAggregateContext(getContextMemPtr(func));
}

//------------------------------------------------------------------------------------
// Results
//------------------------------------------------------------------------------------

template <class T>
AbstractDb2<T>::Results::Results(AbstractDb2<T>* db, sqlite_vm* stmt, bool error) :
    db(db), stmt(stmt)
{
    if (error)
    {
        errorCode = db->lastErrorCode;
        errorMessage = db->lastError;
        return;
    }

    fetchNext();
    if (colCount == 0)
    {
        affected = 0;
    }
    else
    {
        affected = sqlite_changes(db->dbHandle);
        insertRowId["ROWID"] = sqlite_last_insert_rowid(db->dbHandle);
    }
}

template <class T>
AbstractDb2<T>::Results::~Results()
{
    if (!db.isNull())
        db->freeStatement(stmt);
}

template <class T>
QString AbstractDb2<T>::Results::getErrorText()
{
    return errorMessage;
}

template <class T>
int AbstractDb2<T>::Results::getErrorCode()
{
    return errorCode;
}

template <class T>
QStringList AbstractDb2<T>::Results::getColumnNames()
{
    return colNames;
}

template <class T>
int AbstractDb2<T>::Results::columnCount()
{
    return colCount;
}

template <class T>
qint64 AbstractDb2<T>::Results::rowsAffected()
{
    return affected;
}

template <class T>
SqlResultsRowPtr AbstractDb2<T>::Results::nextInternal()
{
    if (!rowAvailable || db.isNull() || !db->isValid(stmt))
        return SqlResultsRowPtr();

    Row* row = new Row;
    row->init(colNames, nextRowValues);

    fetchNext();
    return SqlResultsRowPtr(row);
}

template <class T>
bool AbstractDb2<T>::Results::hasNextInternal()
{
    return rowAvailable;
}

template <class T>
int AbstractDb2<T>::Results::fetchNext()
{
    if (db.isNull() || !db->isValid(stmt))
        rowAvailable = false;

    if (!rowAvailable)
        return SQLITE_MISUSE;

    rowAvailable = false;

    const char** values;
    const char** columns;
    int columnsCount;

    int res;
    int secondsSpent = 0;
    while ((res = sqlite_step(stmt, &columnsCount, &values, &columns)) == SQLITE_BUSY && secondsSpent < db->getTimeout())
    {
        QThread::sleep(1);
        if (db->getTimeout() >= 0)
            secondsSpent++;
    }

    switch (res)
    {
        case SQLITE_ROW:
            rowAvailable = true;
            break;
        case SQLITE_DONE:
            // Empty pointer as no more results are available.
            break;
        default:
            errorCode = res;
            errorMessage = db->freeStatement(stmt);
            return SQLITE_ERROR;
    }

    // First row, initialize members
    if (colCount == -1)
        init(columnsCount, columns);

    // Then read the next row data
    nextRowValues.clear();
    if (rowAvailable)
    {
        for (int i = 0; i < colCount; i++)
            nextRowValues << QString::fromUtf8(values[i]);
    }

    return SQLITE_OK;
}

template <class T>
void AbstractDb2<T>::Results::init(int columnsCount, const char** columns)
{
    colCount = columnsCount;

    TokenList columnDescription;
    for (int i = 0; i < colCount; i++)
    {
        columnDescription = Lexer::tokenize(QString::fromUtf8(columns[i]), Dialect::Sqlite2).filterWhiteSpaces();
        if (columnDescription.size() > 0)
        {
            // If the column is prefixed with dbname and table name, then we remove them.
            for (int j = 0; j < 2 &&columnDescription.size() > 1 && columnDescription[1]->type == Token::OPERATOR && columnDescription[1]->value == "."; j++)
            {
                columnDescription.removeFirst();
                columnDescription.removeFirst();
            }

            colNames << stripObjName(columnDescription.first()->value, Dialect::Sqlite2);
        }
        else
            colNames << "";
    }
}

//------------------------------------------------------------------------------------
// Row
//------------------------------------------------------------------------------------

template <class T>
void AbstractDb2<T>::Results::Row::init(const QStringList& columns, const QList<QVariant>& resultValues)
{
    for (int i = 0; i < columns.size(); i++)
    {
        values << resultValues[i];
        valuesMap[columns[i]] = resultValues[i];
    }
}

#endif // ABSTRACTDB2_H
