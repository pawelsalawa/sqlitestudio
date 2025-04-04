#include "erdchangenewentity.h"
#include "erdentity.h"

ErdChangeNewEntity::ErdChangeNewEntity(ErdEntity* entity, Db* db, const SqliteCreateTablePtr& createTable) :
    ErdChange(Category::ENTITY_NEW, true), entity(entity), db(db), createTable(createTable)
{
}

QStringList ErdChangeNewEntity::getChangeDdl()
{
    createTable->rebuildTokens();
    return {createTable->detokenize()};
}

ErdEntity* ErdChangeNewEntity::getEntity() const
{
    return entity;
}

QString ErdChangeNewEntity::getTableName() const
{
    return createTable->table;
}
