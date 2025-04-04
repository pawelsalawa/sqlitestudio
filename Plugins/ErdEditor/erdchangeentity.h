#ifndef ERDCHANGEENTITY_H
#define ERDCHANGEENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class Db;
class TableModifier;
class ErdEntity;

class ErdChangeEntity : public ErdChange
{
    public:
        ErdChangeEntity(ErdEntity* entity, Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after);

        TableModifier *getTableModifier() const;
        ErdEntity *getEntity() const;
        QString getTableNameBefore() const;
        QString getTableNameAfter() const;

    protected:
        QStringList getChangeDdl();

    private:
        ErdEntity* entity = nullptr;
        Db* db = nullptr;
        SqliteCreateTablePtr before;
        SqliteCreateTablePtr after;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEENTITY_H
