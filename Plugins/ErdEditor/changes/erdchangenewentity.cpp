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

void ErdChangeNewEntity::refreshReferencingTables(ErdScene::SceneChangeApi& api)
{
    QString t = createTable->table;
    QStringList referencingTables = api.schemaResolver().getFkReferencingTables(t);
    for (const QString& tableName : referencingTables)
        api.refreshEntity(tableName, tableName);
}

void ErdChangeNewEntity::apply(ErdScene::SceneChangeApi& api)
{
    api.refreshEntity(temporaryEntityName, createTable->table);
    refreshReferencingTables(api);
}

void ErdChangeNewEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    lastPositionBeforeUndo = api.getEntityPosition(createTable->table);
    api.refreshEntitiesByTableNames({createTable->table});
}

void ErdChangeNewEntity::applyRedo(ErdScene::SceneChangeApi& api)
{
    if (lastPositionBeforeUndo.isNull())
        qWarning() << "Redoing ErdChangeNewEntity for table" << createTable->table
                   << "but lastPositionBeforeUndo was not set.";

    api.refreshEntitiesByTableNames({createTable->table});
    api.setEntityPosition(createTable->table, lastPositionBeforeUndo);
    refreshReferencingTables(api);
}
