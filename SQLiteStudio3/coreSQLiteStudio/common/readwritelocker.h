#ifndef READWRITELOCKER_H
#define READWRITELOCKER_H

#include "coreSQLiteStudio_global.h"
#include "dialect.h"

class QReadLocker;
class QWriteLocker;
class QReadWriteLock;

/**
 * @brief The ReadWriteLocker class
 *
 * This class behaves pretty much like QReadLocker or QWriteLocker
 * (it actually uses those internally), except it can be either
 * of those and this is to be decided at the moment of creation.
 * Therefore the locker can work as read or write locker depending
 * on an external condition.
 * There's also a possibility to not lock anything as a third
 * choice of working mode, so this also can be decided
 * at construction moment.
 */
class API_EXPORT ReadWriteLocker
{
    public:
        enum Mode
        {
            READ,
            WRITE,
            NONE
        };

        ReadWriteLocker(QReadWriteLock* lock, Mode mode);
        ReadWriteLocker(QReadWriteLock* lock, const QString& query, Dialect dialect, bool noLock);
        virtual ~ReadWriteLocker();

        /**
         * @brief Provides required locking mode for given query.
         * @param query Query to be executed.
         * @return Locking mode: READ or WRITE.
         *
         * Given the query this method analyzes what is the query and provides information if the query
         * will do some changes on the database, or not. Then it returns proper locking mode that should
         * be used for this query execution.
         *
         * Query execution methods from this class check if lock mode of the query to be executed isn't
         * in conflict with the lock being currently applied on the dbOperLock (if any is applied at the moment).
         *
         * This method works on a very simple rule. It assumes that queries: SELECT, ANALYZE, EXPLAIN,
         * and PRAGMA - are read-only, while all other queries are read-write.
         * In case of PRAGMA this is not entirely true, but it's not like using PRAGMA for changing
         * some setting would cause database state inconsistency. At least not from perspective of SQLiteStudio.
         *
         * In case of WITH statement it filters out the "WITH clause" and then checks for SELECT keyword.
         */
        static ReadWriteLocker::Mode getMode(const QString& query, Dialect dialect, bool noLock);

    private:
        void init(QReadWriteLock* lock, Mode mode);

        QReadLocker* readLocker = nullptr;
        QWriteLocker* writeLocker = nullptr;
};

#endif // READWRITELOCKER_H
