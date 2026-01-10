#include "erdchangedeleteentity.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "tablemodifier.h"

ErdChangeDeleteEntity::ErdChangeDeleteEntity(Db* db, const QString& tableName, const QPointF& pos, const QColor& customColor, const QString& description) :
    ErdChange(Category::ENTITY_DELETE, description, true), db(db), tableName(tableName),
    lastPosition(pos), lastCustomColor(customColor)
{
}

ErdChangeDeleteEntity::~ErdChangeDeleteEntity()
{
    safe_delete(tableModifier);
}

void ErdChangeDeleteEntity::apply(ErdScene::SceneChangeApi& api)
{
    QStringList tables = tableModifier->getModifiedTables();
    api.refreshEntitiesByTableNames(tables);
    api.removeEntityFromScene(tableName);
}

void ErdChangeDeleteEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    QStringList modifiedTables;
    modifiedTables << tableName;
    modifiedTables += tableModifier->getModifiedTables();

    api.refreshEntitiesByTableNames(modifiedTables);
    api.setEntityPosition(tableName, lastPosition);
    if (lastCustomColor.isValid())
        api.setEntityColor(tableName, lastCustomColor);
}

QStringList ErdChangeDeleteEntity::getChangeDdl()
{
    if (!tableModifier)
    {
        tableModifier = new TableModifier(db, tableName);
        tableModifier->dropTable();
    }
    return tableModifier->generateSqls();
}

