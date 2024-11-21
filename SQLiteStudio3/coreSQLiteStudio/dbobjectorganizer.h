#ifndef DBOBJECTORGANIZER_H
#define DBOBJECTORGANIZER_H

#include "coreSQLiteStudio_global.h"
#include "interruptable.h"
#include "schemaresolver.h"
#include <QString>
#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <QStringList>
#include <QHash>

class Db;

class API_EXPORT DbObjectOrganizer : public QObject, public QRunnable, public Interruptable
{
        Q_OBJECT

    public:
        typedef std::function<bool(const QStringList& tables)> ReferencedTablesConfimFunction;
        typedef std::function<bool(QString& nameInConflict)> NameConflictResolveFunction;
        typedef std::function<bool(const QList<QPair<QString,QString>>& diffs)> ConversionConfimFunction;
        typedef std::function<bool(const QHash<QString,QSet<QString>>& errors)> ConversionErrorsConfimFunction;

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
         * @param conversionConfimFunction This function should display changes that will be made to the SQL statements
         * while executing them on the target database (because of different database versions). User can cancel the process
         * (in which case, the function should return false).
         * @param conversionErrorsConfimFunction This function should display critical errors that occurred when tried to
         * convert SQL statements for target database. User
         * can choose to proceed (with skipping problemating database objects), or cancel the process (in which case the function
         * should return false).
         *
         * Use this constructor always if possible.
         */
        DbObjectOrganizer(ReferencedTablesConfimFunction confirmFunction,
                          NameConflictResolveFunction nameConflictResolveFunction,
                          ConversionConfimFunction conversionConfimFunction,
                          ConversionErrorsConfimFunction conversionErrorsConfimFunction);
        ~DbObjectOrganizer();

        void copyObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers);
        void moveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers);
        void interrupt();
        bool isExecuting();
        void run();

    private:
        enum class Mode
        {
            PREPARE_TO_COPY_OBJECTS,
            PREPARE_TO_MOVE_OBJECTS,
            COPY_OBJECTS,
            MOVE_OBJECTS,
            unknown
        };

        void init();
        void reset();
        void copyOrMoveObjectsToDb(Db* srcDb, const QSet<QString>& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers, bool move);
        void processPreparation();
        bool processAll();
        bool processDbObjects();
        bool processColumns();
        bool resolveNameConflicts();
        bool copyTableToDb(const QString& table);
        bool copyViewToDb(const QString& view);
        bool copyIndexToDb(const QString& index);
        bool copyTriggerToDb(const QString& trigger);
        bool copySimpleObjectToDb(const QString& name, const QString& errorMessage, SchemaResolver::ObjectType objectType);
        void collectReferencedTables(const QString& table);
        void collectReferencedIndexes(const QString& table);
        void collectReferencedTriggersForTable(const QString& table);
        void collectReferencedTriggersForView(const QString& view);
        void findBinaryColumns(const QString& table, const StrHash<SqliteQueryPtr>& allParsedObjects);
        bool copyDataAsMiddleware(const QString& table);
        bool copyDataUsingAttach(const QString& table);
        void dropTable(const QString& table);
        void dropView(const QString& view);
        void dropObject(const QString& name, const QString& type);
        bool setFkEnabled(bool enabled);
        bool isInterrupted();
        void setExecuting(bool executing);
        void setSrcAndDstDb(Db* srcDb, Db* dstDb);
        bool begin();
        bool commit();
        bool rollback();
        void emitFinished(bool success);
        bool execConfirmFunctionInMainThread(const QStringList& tables);
        QString processSimpleObjectAttachNameAndRename(const QString& objName, const QString& ddl);

        ReferencedTablesConfimFunction confirmFunction;
        NameConflictResolveFunction nameConflictResolveFunction;
        ConversionConfimFunction conversionConfimFunction;
        ConversionErrorsConfimFunction conversionErrorsConfimFunction;
        Mode mode = Mode::COPY_OBJECTS;
        Db* srcDb = nullptr;
        Db* dstDb = nullptr;
        QSet<QString> srcNames;
        QSet<QString> srcTables;
        QSet<QString> srcViews;
        QSet<QString> srcIndexes;
        QSet<QString> srcTriggers;
        QHash<QString,QString> renamed;
        QString srcTable;
        QHash<QString,QStringList> binaryColumns;
        bool includeData = false;
        bool includeIndexes = false;
        bool includeTriggers = false;
        bool deleteSourceObjects = false;
        QSet<QString> referencedTables;
        QHash<QString,QSet<QString>> errorsToConfirm;
        QList<QPair<QString, QString>> diffListToConfirm;
        SchemaResolver* srcResolver = nullptr;
        SchemaResolver* dstResolver = nullptr;
        bool interrupted = false;
        bool executing = false;
        QMutex interruptMutex;
        QMutex executingMutex;
        QString attachName;

    private slots:
        void processPreparationFinished();
        bool confirmFunctionSlot(const QStringList& tables);

    signals:
        void finishedDbObjectsMove(bool success, Db* srcDb, Db* dstDb);
        void finishedDbObjectsCopy(bool success, Db* srcDb, Db* dstDb);
        void preparetionFinished();
};

#endif // DBOBJECTORGANIZER_H
