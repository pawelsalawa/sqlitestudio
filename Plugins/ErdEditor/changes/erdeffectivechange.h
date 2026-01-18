#ifndef ERDEFFECTIVECHANGE_H
#define ERDEFFECTIVECHANGE_H

#include "parser/ast/sqlitecreatetable.h"

class ErdEffectiveChange
{
    public:
        enum Type
        {
            MODIFY,
            DROP,
            CREATE,
            NOOP, // to reflect no effective change, e.g., when a created table is later dropped
            INVALID
        };

        ErdEffectiveChange() = default;

        static ErdEffectiveChange noop();
        static ErdEffectiveChange drop(const QString& tableName, const QString& description);
        static ErdEffectiveChange create(const SqliteCreateTablePtr& after, const QString& description);
        static ErdEffectiveChange modify(const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description);

        Type getType() const;
        QString getTableName() const;
        SqliteCreateTablePtr getBefore() const;
        SqliteCreateTablePtr getAfter() const;
        QString getDescription() const;
        bool isValid() const;
        QString getId() const;

    private:
        ErdEffectiveChange(Type type, const QString& description);

        Type type = INVALID;
        QString tableName;
        SqliteCreateTablePtr before;
        SqliteCreateTablePtr after;
        QString description;
        QString id;
};

#endif // ERDEFFECTIVECHANGE_H
