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
        ErdChangeEntity(ErdEntity* entity, Db* db, SqliteCreateTablePtr before, SqliteCreateTablePtr after);

        QString toDdl() const;

    private:
        ErdEntity* entity = nullptr;
        Db* db = nullptr;
        SqliteCreateTablePtr before;
        SqliteCreateTablePtr after;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEENTITY_H
