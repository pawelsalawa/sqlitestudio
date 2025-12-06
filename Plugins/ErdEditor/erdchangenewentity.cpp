#include "erdchangenewentity.h"
#include "erdentity.h"

ErdChangeNewEntity::ErdChangeNewEntity(Db* db, const SqliteCreateTablePtr& createTable) :
    ErdChange(Category::ENTITY_NEW, true), db(db), createTable(createTable)
{
}

QStringList ErdChangeNewEntity::getChangeDdl()
{
    createTable->rebuildTokens();
    return {createTable->detokenize()};
}

QString ErdChangeNewEntity::getTableName() const
{
    return createTable->table;
}
