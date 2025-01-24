#ifndef ERDCHANGENEWENTITY_H
#define ERDCHANGENEWENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class ErdEntity;
class Db;

class ErdChangeNewEntity : public ErdChange
{
    public:
        ErdChangeNewEntity(ErdEntity* entity, Db* db, const SqliteCreateTablePtr& createTable);

        QStringList toDdl();
        ErdEntity *getEntity() const;
        QString getTableName() const;

    private:
        ErdEntity* entity = nullptr;
        Db* db = nullptr;
        SqliteCreateTablePtr createTable;
};

#endif // ERDCHANGENEWENTITY_H
