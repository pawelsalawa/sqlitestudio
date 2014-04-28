#ifndef DBOBJECTORGANIZER_H
#define DBOBJECTORGANIZER_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <QStringList>
#include <QHash>

class Db;
class SchemaResolver;

class API_EXPORT DbObjectOrganizer : public QObject, public QRunnable
{
        Q_OBJECT

    public:
        typedef std::function<bool(const QStringList& tables)> ReferencedTablesConfimFunction;
        typedef std::function<bool(QString& nameInConflict)> NameConflictResolveFunction;

        /**
         * @brief Creates organizer with default handler functions.
         *
         * The default handler functions are not very usable - they always return false.
         * It's better to use the other constructor and pass the custom implementation of
         * handler functions.
         */
        DbObjectOrganizer();

        /**
         * @brief Creates organizer with handler functions defined specially for it.
         * @param confirmFunction Implemented function should ask user if he wants to include referenced tables
         * in the copy/move action. If it returns false, then referenced tables will be excluded.
         * @param nameConflictResolveFunction Implemented function should ask user for a new name for the object
         * with the name passed in the argument. The new name should be returned in the very same reference argument.
         * If the function returns false, then the conflict is unresolved and copy/move action will be aborted.
         * When the function returns true, it means that the user entered a new name and he wants to proceed.
         *
         * Use this constructor always if possible.
         */
        DbObjectOrganizer(ReferencedTablesConfimFunction confirmFunction,
                          NameConflictResolveFunction nameConflictResolveFunction);
        ~DbObjectOrganizer();

        void copyObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData);
        void moveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData);
        void copyColumnsToTable(Db* srcDb, const QString& srcTable, const QStringList& columnNames, Db* dstDb,
                                const QString& dstTable, bool includeData);
        void moveColumnsToTable(Db* srcDb, const QString& srcTable, const QStringList& columnNames, Db* dstDb,
                                const QString& dstTable, bool includeData);
        void interrupt();
        void run();

    protected:
        enum class Mode
        {
            COPY_OBJECTS,
            MOVE_OBJECTS,
            COPY_COLUMNS,
            MOVE_COLUMNS,
            unknown
        };

        void reset();
        void copyOrMoveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool move);
        void copyOrMoveColumnsToTable(Db* srcDb, const QString& srcTable, const QStringList& columnNames, Db* dstDb,
                                      const QString& dstTable, bool includeData, bool move);
        bool processAll();
        bool processAllObjects();
        bool processColumns();
        bool resolveNameConflicts();
        bool copyTableToDb(const QString& table);
        bool copyViewToDb(const QString& view);
        QStringList resolveReferencedtables(const QString& table);
        void collectReferencedTables(const QString& table);
        bool copyDataAsMiddleware(const QString& table);
        bool copyDataUsingAttach(const QString& table, const QString& attachName);
        void dropTable(const QString& table);
        void dropView(const QString& view);
        bool setFkEnabled(bool enabled);
        bool isInterrupted();
        void setSrcAndDstDb(Db* srcDb, Db* dstDb);
        bool begin();
        bool commit();
        bool rollback();
        void emitFinished(bool success);

        ReferencedTablesConfimFunction confirmFunction;
        NameConflictResolveFunction nameConflictResolveFunction;
        Mode mode = Mode::COPY_OBJECTS;
        Db* srcDb = nullptr;
        Db* dstDb = nullptr;
        QStringList srcNames;
        QStringList srcTables;
        QStringList srcViews;
        QHash<QString,QString> renamedObjects;
        QString srcTable;
        QString dstTable;
        bool includeData = false;
        bool deleteSourceObjects = false;
        QStringList referencedTables;
        SchemaResolver* srcResolver = nullptr;
        SchemaResolver* dstResolver = nullptr;
        bool interrupted = false;
        QMutex interruptMutex;

    signals:
        void finishedDbObjectsMove(bool success, Db* srcDb, Db* dstDb);
        void finishedDbObjectsCopy(bool success, Db* srcDb, Db* dstDb);
        void finishedColumnsMove(bool success, Db* srcDb, Db* dstDb, const QString& srcTable, const QString& dstTable);
        void finishedColumnsCopy(bool success, Db* srcDb, Db* dstDb, const QString& srcTable, const QString& dstTable);
};

#endif // DBOBJECTORGANIZER_H
