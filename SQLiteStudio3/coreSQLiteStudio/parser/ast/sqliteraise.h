#ifndef SQLITERAISE_H
#define SQLITERAISE_H

#include "sqlitestatement.h"
#include <QString>

class SqliteExpr;

class API_EXPORT SqliteRaise : public SqliteStatement
{
    Q_OBJECT

    public:
        enum class Type
        {
            IGNORE,
            ROLLBACK,
            ABORT,
            FAIL,
            null
        };

        SqliteRaise();
        SqliteRaise(const SqliteRaise& other);
        explicit SqliteRaise(const QString& type);
        SqliteRaise(const QString& type, SqliteExpr* value);

        SqliteStatement* clone();

        static Type raiseType(const QString& value);
        static QString raiseType(Type value);

        Type type = Type::null;
        SqliteExpr* expr = nullptr;

    protected:
        TokenList rebuildTokensFromContents() const;
};

typedef QSharedPointer<SqliteRaise> SqliteRaisePtr;

#endif // SQLITERAISE_H
