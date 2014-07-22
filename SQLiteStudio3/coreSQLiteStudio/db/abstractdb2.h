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
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        void initAfterOpen();
        SqlQueryPtr prepare(const QString& query);
        QString getTypeLabel();
        bool deregisterFunction(const QString& name, int argCount);
        bool registerScalarFunction(const QString& name, int argCount);
        bool registerAggregateFunction(const QString& name, int argCount);
        bool registerCollationInternal(const QString& name);
        bool deregisterCollationInternal(const QString& name);

    private:
        class Query : public SqlQuery, public Sqlite2ColumnDataTypeHelper
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        void init(const QStringList& columns, const QList<QVariant>& resultValues);
                };

                Query(AbstractDb2<T>* db, const QString& query);
                ~Query();

                QString getErrorText();
                int getErrorCode();
                QStringList getColumnNames();
                int columnCount();
                qint64 rowsAffected();
                QString finalize();

            protected:
                SqlResultsRowPtr nextInternal();
                bool hasNextInternal();
                bool execInternal(const QList<QVariant>& args);
                bool execInternal(const QHash<QString, QVariant>& args);

            private:
                int prepareStmt(const QString& processedQuery);
                int resetStmt();
                int bindParam(int paramIdx, const QVariant& value);
                int fetchNext();
                int fetchFirst();
                void init(int columnsCount, const char** columns);
                bool checkDbState();
                void copyErrorFromDb();
                void copyErrorToDb();
                void setError(int code, const QString& msg);

                static QString replaceNamedParams(const QString& query);

                QPointer<AbstractDb2<T>> db;
                sqlite_vm* stmt = nullptr;
                int errorCode = SQLITE_OK;
                QString errorMessage;
                int colCount = -1;
                QStringList colNames;
                QList<QVariant> nextRowValues;
                int affected = 0;
                bool rowAvailable = false;
        };

        void cleanUp();
        void resetError();
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

        sqlite* dbHandle = nullptr;
        QString dbErrorMessage;
        int dbErrorCode = SQLITE_OK;
        QList<FunctionUserData*> userDataList;
        QList<Query*> queries;
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
SqlQueryPtr AbstractDb2<T>::prepare(const QString& query)
{
    return SqlQueryPtr(new Query(this, query));
}

template <class T>
void AbstractDb2<T>::interruptExecution()
{
    if (!isOpenInternal())
        return;

    sqlite_interrupt(dbHandle);
}

template <class T>
QString AbstractDb2<T>::getErrorTextInternal()
{
    return dbErrorMessage;
}

template <class T>
int AbstractDb2<T>::getErrorCodeInternal()
{
    return dbErrorCode;
}

template <class T>
bool AbstractDb2<T>::openInternal()
{
    resetError();
    sqlite* handle;
    char* errMsg = nullptr;
    handle = sqlite_open(path.toUtf8().constData(), 0, &errMsg);
    if (!handle)
    {
        dbErrorCode = SQLITE_ERROR;

        if (errMsg)
        {
            dbErrorMessage = tr("Could not open database: %1").arg(QString::fromUtf8(errMsg));
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
    resetError();
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
void AbstractDb2<T>::cleanUp()
{
    for (Query* q : queries)
        q->finalize();
}

template <class T>
void AbstractDb2<T>::resetError()
{
    dbErrorCode = 0;
    dbErrorMessage = QString::null;
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
        case QVariant::List:
        {
            QList<QVariant> list = result.toList();
            QStringList strList;
            for (const QVariant& v : list)
                strList << v.toString();

            QByteArray ba = strList.join(" ").toUtf8();
            sqlite_set_result_string(func, ba.constData(), ba.size());
            break;
        }
        case QVariant::StringList:
        {
            QByteArray ba = result.toStringList().join(" ").toUtf8();
            sqlite_set_result_string(func, ba.constData(), ba.size());
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
// Query
//------------------------------------------------------------------------------------

template <class T>
AbstractDb2<T>::Query::Query(AbstractDb2<T>* db, const QString& query) :
    db(db)
{
    this->query = query;
    db->queries << this;
}

template <class T>
AbstractDb2<T>::Query::~Query()
{
    if (db.isNull())
        return;

    finalize();
    db->queries.removeOne(this);
}

template <class T>
void AbstractDb2<T>::Query::copyErrorFromDb()
{
    if (db->dbErrorCode != 0)
    {
        errorCode = db->dbErrorCode;
        errorMessage = db->dbErrorMessage;
        return;
    }
}

template <class T>
void AbstractDb2<T>::Query::copyErrorToDb()
{
    db->dbErrorCode = errorCode;
    db->dbErrorMessage = errorMessage;
}

template <class T>
void AbstractDb2<T>::Query::setError(int code, const QString& msg)
{
    if (errorCode != SQLITE_OK)
        return; // don't overwrite first error

    errorCode = code;
    errorMessage = msg;
    copyErrorToDb();
}

template <class T>
int AbstractDb2<T>::Query::prepareStmt(const QString& processedQuery)
{
    char* errMsg = nullptr;
    const char* tail;
    QByteArray queryBytes = processedQuery.toUtf8();
    int res = sqlite_compile(db->dbHandle, queryBytes.constData(), &tail, &stmt, &errMsg);
    if (res != SQLITE_OK)
    {
        finalize();
        if (errMsg)
        {
            setError(res, QString::fromUtf8((errMsg)));
            sqlite_freemem(errMsg);
        }
        return res;
    }

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return SQLITE_OK;
}

template <class T>
int AbstractDb2<T>::Query::resetStmt()
{
    errorCode = 0;
    errorMessage = QString::null;
    affected = 0;
    colCount = -1;
    rowAvailable = false;
    nextRowValues.clear();

    char* errMsg = nullptr;
    int res = sqlite_reset(stmt, &errMsg);
    if (res != SQLITE_OK)
    {
        stmt = nullptr;
        if (errMsg)
        {
            setError(res, QString::fromUtf8((errMsg)));
            sqlite_freemem(errMsg);
        }
        return res;
    }
    return SQLITE_OK;
}

template <class T>
bool AbstractDb2<T>::Query::execInternal(const QList<QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite2, flags.testFlag(Db::Flag::NO_LOCK));

    QueryWithParamCount queryWithParams = getQueryWithParamCount(query, Dialect::Sqlite2);
    QString singleStr = replaceNamedParams(queryWithParams.first);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt(singleStr);

    if (res != SQLITE_OK)
        return false;

    for (int paramIdx = 1; paramIdx <= queryWithParams.second; paramIdx++)
    {
        res = bindParam(paramIdx, args[paramIdx-1]);
        if (res != SQLITE_OK)
            return false;
    }

    bool ok = (fetchFirst() == SQLITE_OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
bool AbstractDb2<T>::Query::execInternal(const QHash<QString, QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite2, flags.testFlag(Db::Flag::NO_LOCK));

    QueryWithParamNames queryWithParams = getQueryWithParamNames(query, Dialect::Sqlite2);
    QString singleStr = replaceNamedParams(queryWithParams.first);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt(singleStr);

    if (res != SQLITE_OK)
        return false;

    int paramIdx = 1;
    foreach (const QString& paramName, queryWithParams.second)
    {
        if (!args.contains(paramName))
        {
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        res = bindParam(paramIdx++, args[paramName]);
        if (res != SQLITE_OK)
            return false;
    }

    bool ok = (fetchFirst() == SQLITE_OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
QString AbstractDb2<T>::Query::replaceNamedParams(const QString& query)
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
int AbstractDb2<T>::Query::bindParam(int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return sqlite_bind(stmt, paramIdx, nullptr, 0, 0);
    }

    switch (value.type())
    {
        case QVariant::ByteArray:
        {
            // NOTE: SQLite 2 has a bug that makes it impossible to write BLOB with nulls inside. First occurrance of the null
            // makes the whole value to be saved as truncated to that position. Nothing I can do about it.
            QByteArray ba = value.toByteArray();
            return sqlite_bind(stmt, paramIdx, ba.constData(), ba.size(), true);
        }
        default:
        {
            QByteArray ba = value.toString().toUtf8();
            ba.append('\0');
            return sqlite_bind(stmt, paramIdx, ba.constData(), ba.size(), true);
        }
    }

    return SQLITE_MISUSE; // not going to happen
}
template <class T>
QString AbstractDb2<T>::Query::getErrorText()
{
    return errorMessage;
}

template <class T>
int AbstractDb2<T>::Query::getErrorCode()
{
    return errorCode;
}

template <class T>
QStringList AbstractDb2<T>::Query::getColumnNames()
{
    return colNames;
}

template <class T>
int AbstractDb2<T>::Query::columnCount()
{
    return colCount;
}

template <class T>
qint64 AbstractDb2<T>::Query::rowsAffected()
{
    return affected;
}

template <class T>
SqlResultsRowPtr AbstractDb2<T>::Query::nextInternal()
{
    if (!rowAvailable || db.isNull())
        return SqlResultsRowPtr();

    Row* row = new Row;
    row->init(colNames, nextRowValues);

    int res = fetchNext();
    if (res != SQLITE_OK)
    {
        delete row;
        return SqlResultsRowPtr();
    }
    return SqlResultsRowPtr(row);
}

template <class T>
bool AbstractDb2<T>::Query::hasNextInternal()
{
    return rowAvailable && stmt;
}

template <class T>
int AbstractDb2<T>::Query::fetchFirst()
{
    rowAvailable = true;
    int res = fetchNext();
    if (res == SQLITE_OK)
    {
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
    return res;
}

template <class T>
bool AbstractDb2<T>::Query::checkDbState()
{
    if (db.isNull() || !db->dbHandle)
    {
        setError(SqlErrorCode::DB_NOT_DEFINED, "SqlQuery is no longer valid.");
        return false;
    }

    return true;
}

template <class T>
QString AbstractDb2<T>::Query::finalize()
{
    QString msg;
    if (stmt)
    {
        char* errMsg = nullptr;
        sqlite_finalize(stmt, &errMsg);
        stmt = nullptr;
        if (errMsg)
        {
            msg = QString::fromUtf8(errMsg);
            sqlite_freemem(errMsg);
        }
    }
    return msg;
}

template <class T>
int AbstractDb2<T>::Query::fetchNext()
{
    if (!checkDbState())
        rowAvailable = false;

    if (!rowAvailable || !stmt)
    {
        setError(SQLITE_MISUSE, tr("Result set expired or no row available."));
        return SQLITE_MISUSE;
    }

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
            setError(res, finalize());
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
        {
            if (isBinaryColumn(i))
                nextRowValues << QByteArray(values[i]);
            else
                nextRowValues << QString::fromUtf8(values[i]);
        }
    }

    return SQLITE_OK;
}

template <class T>
void AbstractDb2<T>::Query::init(int columnsCount, const char** columns)
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
void AbstractDb2<T>::Query::Row::init(const QStringList& columns, const QList<QVariant>& resultValues)
{
    for (int i = 0; i < columns.size(); i++)
    {
        values << resultValues[i];
        valuesMap[columns[i]] = resultValues[i];
    }
}

#endif // ABSTRACTDB2_H
