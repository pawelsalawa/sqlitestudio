#ifndef SQLITECREATETRIGGER_H
#define SQLITECREATETRIGGER_H

#include "sqliteddlwithdbcontext.h"
#include "sqlitequery.h"
#include "sqlitetablerelatedddl.h"

#include <QString>
#include <QList>

class SqliteExpr;

class API_EXPORT SqliteCreateTrigger : public SqliteQuery, public SqliteTableRelatedDdl, public SqliteDdlWithDbContext
{
    Q_OBJECT

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
                SqliteStatement* clone();

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
        SqliteStatement* clone();

        QString getTargetTable() const;
        QString getTargetDatabase() const;
        void setTargetDatabase(const QString& database);
        QString getObjectName() const;
        void setObjectName(const QString& name);

        bool tempKw = false;
        bool temporaryKw = false;
        bool ifNotExistsKw = false;
        QString database = QString();
        QString trigger = QString();
        QString table = QString(); // can also be a view name
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
