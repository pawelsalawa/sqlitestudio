#include "erdchangenewentity.h"
#include "scene/erdentity.h"

ErdChangeNewEntity::ErdChangeNewEntity(Db* db, const QString& temporaryEntityName, const SqliteCreateTablePtr& createTable, const QString& description) :
    ErdChange(Category::ENTITY_NEW, description, true), db(db), temporaryEntityName(temporaryEntityName), createTable(createTable)
{
}

QStringList ErdChangeNewEntity::getChangeDdl()
{
    createTable->rebuildTokens();
    return {createTable->detokenize()};
}

void ErdChangeNewEntity::setLastPositionBeforeUndo(QPointF pos)
{
    lastPositionBeforeUndo = pos;
}

QPointF ErdChangeNewEntity::getLastPositionBeforeUndo() const
{
    return lastPositionBeforeUndo;
}

QString ErdChangeNewEntity::getTemporaryEntityName() const
{
    return temporaryEntityName;
}

QString ErdChangeNewEntity::getTableName() const
{
    return createTable->table;
}
