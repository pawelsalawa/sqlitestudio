#ifndef SQLITECREATETRIGGER_H
#define SQLITECREATETRIGGER_H

#include "sqlitequery.h"
#include "sqlitetablerelatedddl.h"

#include <QString>
#include <QList>

class SqliteExpr;

class API_EXPORT SqliteCreateTrigger : public SqliteQuery, public SqliteTableRelatedDdl
{
    public:
        enum class Time
        {
            BEFORE,
            AFTER,
            INSTEAD_OF,
            null
        };

        class API_EXPORT Event : public SqliteStatement
        {
            public:
                enum Type
                {
                    INSERT,
                    UPDATE,
                    DELETE,
                    UPDATE_OF,
                    null
                };

                Event();
                explicit Event(Type type);
                Event(const Event& other);
                explicit Event(const QList<QString>& columns);
                TokenList rebuildTokensFromContents();

                static QString typeToString(Type type);
                static Type stringToType(const QString& type);

                Type type;
                QStringList columnNames;
        };

        enum class Scope
        {
            FOR_EACH_ROW,
            FOR_EACH_STATEMENT, // Sqlite2 only
            null
        };

        SqliteCreateTrigger();
        SqliteCreateTrigger(const SqliteCreateTrigger& other);
        SqliteCreateTrigger(int temp, bool ifNotExists, const QString& name1, const QString& name2,
                            const QString& name3, Time time, Event* event, Scope scope,
                            SqliteExpr* precondition, const QList<SqliteQuery*>& queries, int sqliteVersion);
        ~SqliteCreateTrigger();

        QString getTargetTable() const;

        bool tempKw = false;
        bool temporaryKw = false;
        bool ifNotExistsKw = false;
        // The database refers to the trigger name in Sqlite3, but in Sqlite2 it refers to the table.
        QString database = QString::null;
        QString trigger = QString::null;
        QString table = QString::null; // can also be a view name
        Event* event = nullptr;
        Time eventTime = Time::null;
        Scope scope = Scope::null;
        SqliteExpr* precondition = nullptr;
        QList<SqliteQuery*> queries;

        static QString time(Time eventTime);
        static Time time(const QString& eventTime);
        static QString scopeToString(Scope scope);
        static Scope stringToScope(const QString& scope);

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCreateTrigger> SqliteCreateTriggerPtr;

#endif // SQLITECREATETRIGGER_H
