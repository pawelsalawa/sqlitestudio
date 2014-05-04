#ifndef SCHEMARESOLVER_H
#define SCHEMARESOLVER_H

#include "parser/parser.h"
#include "parser/ast/sqlitequerytype.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "coreSQLiteStudio_global.h"
#include "selectresolver.h"
#include <QStringList>

class Db;
class SqliteCreateTable;
class SqliteQuery;

// TODO add cache

class API_EXPORT SchemaResolver
{
    public:
        struct ObjectDetails
        {
            enum Type
            {
                TABLE,
                INDEX,
                TRIGGER,
                VIEW
            };

            Type type;
            QString ddl;
        };

        explicit SchemaResolver(Db* db);
        virtual ~SchemaResolver();

        QStringList getTables(const QString& database = QString::null);
        QStringList getIndexes(const QString& database = QString::null);
        QStringList getTriggers(const QString& database = QString::null);
        QStringList getViews(const QString& database = QString::null);
        QMap<QString, QStringList> getGroupedIndexes(const QString& database = QString::null);
        QMap<QString, QStringList> getGroupedTriggers(const QString& database = QString::null);
        QSet<QString> getDatabases();
        QStringList getObjects(const QString& type);
        QStringList getObjects(const QString& database, const QString& type);
        QStringList getAllObjects();
        QStringList getAllObjects(const QString& database);
        QString getUniqueName(const QString& database, const QString& namePrefix);
        QString getUniqueName(const QString& namePrefix = QString::null);
        QStringList getFkReferencingTables(const QString& table);
        QStringList getFkReferencingTables(const QString& database, const QString& table);

        QStringList getIndexesForTable(const QString& database, const QString& table);
        QStringList getIndexesForTable(const QString& table);
        QStringList getIndexesForView(const QString& database, const QString& table);
        QStringList getIndexesForView(const QString& table);
        QStringList getTriggersForTable(const QString& database, const QString& table);
        QStringList getTriggersForTable(const QString& table);
        QStringList getViewsForTable(const QString& database, const QString& table);
        QStringList getViewsForTable(const QString& table);

        QHash<QString,ObjectDetails> getAllObjectDetails();
        QHash<QString,ObjectDetails> getAllObjectDetails(const QString& database);

        QList<SqliteCreateIndexPtr> getParsedIndexesForTable(const QString& database, const QString& table);
        QList<SqliteCreateIndexPtr> getParsedIndexesForTable(const QString& table);
        QList<SqliteCreateTriggerPtr> getParsedTriggersForTable(const QString& database, const QString& table, bool includeContentReferences = false);
        QList<SqliteCreateTriggerPtr> getParsedTriggersForTable(const QString& table, bool includeContentReferences = false);
        QList<SqliteCreateViewPtr> getParsedViewsForTable(const QString& database, const QString& table);
        QList<SqliteCreateViewPtr> getParsedViewsForTable(const QString& table);
        QList<SqliteCreateTriggerPtr> getParsedTriggersForView(const QString& database, const QString& view, bool includeContentReferences = false);
        QList<SqliteCreateTriggerPtr> getParsedTriggersForView(const QString& view, bool includeContentReferences = false);

        bool isWithoutRowIdTable(const QString& table);
        bool isWithoutRowIdTable(const QString& database, const QString& table);
        bool isVirtualTable(const QString& database, const QString& table);
        bool isVirtualTable(const QString& table);

        QStringList getWithoutRowIdTableColumns(const QString& table);
        QStringList getWithoutRowIdTableColumns(const QString& database, const QString& table);

        /**
         * @brief getTableColumns Get column names for a table.
         * @param table Table to query.
         * @return List of column names of the table.
         * This does something similar to getting list of columns with
         * PRAGMA table_info(), but the pragma in Sqlite2 doesn't support
         * queries for attached databases, therefore this method is provided
         * to make this possible. It finds table's DDL and parses it,
         * then extracts list of columns from parsing results.
         */
        QStringList getTableColumns(const QString& table);

        /**
         * @brief getTableColumns Get column names for a table.
         * @param database Attached database name.
         * @param table Table to query.
         * @return List of column names of the table.
         * @overload
         */
        QStringList getTableColumns(const QString& database, const QString& table);
        QHash<QString, QStringList> getAllTableColumns(const QString& database = QString::null);

        QStringList getViewColumns(const QString& view);
        QStringList getViewColumns(const QString& database, const QString& view);
        QList<SelectResolver::Column> getViewColumnObjects(const QString& view);
        QList<SelectResolver::Column> getViewColumnObjects(const QString& database, const QString& view);

        QString getObjectDdl(const QString& name);
        QString getObjectDdl(const QString& database, const QString& name);

        /**
         * @brief Parses given object's DDL.
         * @param name Name of the object in the database.
         * @return Parsed object, or null pointer if named object was not in the database, or parsing error occured.
         *
         * Returned query has to be deleted outside!
         */
        SqliteQueryPtr getParsedObject(const QString& name);

        /**
         * @brief Parses given object's DDL.
         * @param database Database that the object is in (the attach name of the database).
         * @param name Name of the object in the database.
         * @return Parsed object, or null pointer if named object was not in the database, or parsing error occured.
         * @overload
         */
        SqliteQueryPtr getParsedObject(const QString& database, const QString& name);

        QHash<QString,SqliteQueryPtr> getAllParsedObjects();
        QHash<QString,SqliteQueryPtr> getAllParsedObjects(const QString& database);

        static QString getSqliteMasterDdl(bool temp = false);

        QStringList getCollations();

        bool getIgnoreSystemObjects() const;
        void setIgnoreSystemObjects(bool value);

    private:
        SqliteQueryPtr getParsedDdl(const QString& ddl);
        SqliteCreateTablePtr virtualTableAsRegularTable(const QString& database, const QString& table);
        QMap<QString, QStringList> getGroupedObjects(const QString &database, const QStringList& inputList, SqliteQueryType type);
        bool isFilteredOut(const QString& value, const QString& type);
        QList<SqliteCreateTriggerPtr> getParsedTriggersForTableOrView(const QString& database, const QString& tableOrView,
                                                                      bool includeContentReferences, bool table);
        void filterSystemIndexes(QStringList& indexes);

        Db* db = nullptr;
        Parser* parser;
        bool ignoreSystemObjects = false;
};

#endif // SCHEMARESOLVER_H
