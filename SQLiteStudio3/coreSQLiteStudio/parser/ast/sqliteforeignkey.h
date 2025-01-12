#ifndef SQLITEFOREIGNKEY_H
#define SQLITEFOREIGNKEY_H

#include "sqlitestatement.h"
#include "sqliteindexedcolumn.h"
#include "sqlitedeferrable.h"
#include "parser/statementtokenbuilder.h"
#include <QList>
#include <QString>

class API_EXPORT SqliteForeignKey : public SqliteStatement
{
    Q_OBJECT

    public:
        class API_EXPORT Condition : public SqliteStatement
        {
            public:
                enum Action
                {
                    UPDATE,
                    INSERT,
                    DELETE,
                    MATCH
                };

                enum Reaction
                {
                    SET_NULL,
                    SET_DEFAULT,
                    CASCADE,
                    RESTRICT,
                    NO_ACTION
                };

                Condition(Action action, Reaction reaction);
                explicit Condition(const QString& name);
                Condition(const Condition& other);

                static QString toString(Reaction reaction);
                static Reaction toEnum(const QString& reaction);

                SqliteStatement* clone();

                Action action;
                QString name = QString();
                Reaction reaction = NO_ACTION;

            protected:
                TokenList rebuildTokensFromContents();

            private:
                void applyReactionToBuilder(StatementTokenBuilder& builder);
        };

        SqliteForeignKey();
        SqliteForeignKey(const SqliteForeignKey& other);
        ~SqliteForeignKey();

        SqliteStatement* clone();

        QString foreignTable = QString();
        QList<SqliteIndexedColumn*> indexedColumns;
        QList<Condition*> conditions;
        SqliteDeferrable deferrable = SqliteDeferrable::null; // Those two are for table constraint only,
        SqliteInitially initially = SqliteInitially::null;    // because column has its own fields for that.

    protected:
        QStringList getTablesInStatement();
        TokenList getTableTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteForeignKey> SqliteForeignKeyPtr;

#endif // SQLITEFOREIGNKEY_H
