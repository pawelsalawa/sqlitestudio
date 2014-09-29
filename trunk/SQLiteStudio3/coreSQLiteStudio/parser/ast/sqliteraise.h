#ifndef SQLITERAISE_H
#define SQLITERAISE_H

#include "sqlitestatement.h"
#include <QString>

class API_EXPORT SqliteRaise : public SqliteStatement
{
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
        SqliteRaise(const QString& type, const QString& text);

        SqliteStatement* clone();

        static Type raiseType(const QString& value);
        static QString raiseType(Type value);

        Type type = Type::null;
        QString message = QString::null;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteRaise> SqliteRaisePtr;

#endif // SQLITERAISE_H
