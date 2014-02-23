#ifndef ASYNCQUERYRUNNER_H
#define ASYNCQUERYRUNNER_H

#include "db.h"

#include <QVariant>
#include <QHash>
#include <QRunnable>
#include <QString>
#include <QPointer>
#include <QByteArray>

/**
 * @brief Direct query executor to be run in a thread.
 *
 * It's an implementation of QRunnable (so it can be run simply within QThread),
 * that takes query string and arguments for the query and executes the query
 * in separate thread (the one that is owning the runner).
 *
 * The runner is not deleted automatically. Instead the slot for finished() signal
 * has to delete it. It's done like that because the slot will also be interested
 * in execution results and asyncId, before the runner gets deleted.
 *
 * What it does is simply execute Db::exec() from another thread.
 *
 * It's a kind of helper class that is used to implement Db::asyncExec().
 */
class AsyncQueryRunner : public QObject, public QRunnable
{
    Q_OBJECT

    public:
        /**
         * @brief Creates runner and defines basic parameters.
         * @param query Query string to be executed.
         * @param args Parameters to the query (can be either QHash or QList).
         * @param flags Execution flags, that will be later passed to Db::exec().
         *
         * It's not enough to just create runner. You also need to define db with setDb()
         * and asyncId with setAsyncId().
         */
        AsyncQueryRunner(const QString& query, const QVariant& args, Db::Flags flags);

        /**
         * @brief Executes query.
         *
         * This is the major method inherited from QRunnable. It's called from another thread
         * and it executes the query.
         */
        void run();

        /**
         * @brief Provides result from execution.
         * @return Execution results.
         */
        SqlResultsPtr getResults();

        /**
         * @brief Defines database for execution.
         * @param db Database object.
         */
        void setDb(Db* db);

        /**
         * @brief Defines asynchronous ID for this execution.
         * @param id Unique ID.
         */
        void setAsyncId(quint32 id);

        /**
         * @brief Provides previously defined asynchronous ID.
         * @return Unique asynchronous ID.
         */
        quint32 getAsyncId();

    private:
        /**
         * @brief Initializes default values.
         */
        void init();

        /**
         * @brief Database to execute the query on.
         */
        Db* db = nullptr;

        /**
         * @brief Query to execute.
         */
        QString query;

        /**
         * @brief Results from execution.
         */
        SqlResultsPtr results;

        /**
         * @brief Parameters for execution.
         *
         * It's either QList<QVariant> or QHash<QString,QVariant>. If it's anything else,
         * then no execution will be performed and critical error will be logged.
         */
        QVariant args;

        /**
         * @brief The unique asynchronous ID for this query execution.
         */
        quint32 asyncId;

        /**
         * @brief Execution flags passed to Db::exec().
         */
        Db::Flags flags;

    signals:
        /**
         * @brief Emitted after the runner has finished its job.
         *
         * Slot connected to this signal should at least delete the runner,
         * but it can also extract execution results.
         */
        void finished(AsyncQueryRunner*);
};


#endif // ASYNCQUERYRUNNER_H
