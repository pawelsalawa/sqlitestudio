#include "erdchangenewentity.h"
#include "erdentity.h"

ErdChangeNewEntity::ErdChangeNewEntity(Db* db, const QString& temporaryEntityName, const SqliteCreateTablePtr& createTable, const QString& description) :
    ErdChange(Category::ENTITY_NEW, description, true), db(db), temporaryEntityName(temporaryEntityName), createTable(createTable)
{
}

QStringList ErdChangeNewEntity::getChangeDdl()
{
    createTable->rebuildTokens();
    return {createTable->detokenize()};
}

QString ErdChangeNewEntity::getTemporaryEntityName() const
{
    return temporaryEntityName;
}

QString ErdChangeNewEntity::getTableName() const
{
    return createTable->table;
}
