#ifndef SQLITEBEGINTRANS_H
#define SQLITEBEGINTRANS_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include <QString>

class API_EXPORT SqliteBeginTrans : public SqliteQuery
{
    Q_OBJECT

    public:
        enum class Type
        {
            null,
            DEFERRED,
            IMMEDIATE,
            EXCLUSIVE
        };

        SqliteBeginTrans();
        SqliteBeginTrans(const SqliteBeginTrans& other);
        SqliteBeginTrans(Type type, bool transactionKw, const QString& name);
        SqliteBeginTrans(bool transactionKw, const QString& name);
        SqliteStatement* clone();

        QString name; // in docs sqlite2 only, but in gramma it's also sqlite3
        bool transactionKw = false;
        Type type = Type::null; // sqlite3 only

        static QString typeToString(Type type);

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteBeginTrans> SqliteBeginTransPtr;

#endif // SQLITEBEGINTRANS_H
