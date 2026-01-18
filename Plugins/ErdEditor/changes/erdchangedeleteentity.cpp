#include "erdchangedeleteentity.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "erdeffectivechange.h"
#include "tablemodifier.h"

ErdChangeDeleteEntity::ErdChangeDeleteEntity(Db* db, const QString& tableName, const QPointF& pos, const QColor& customColor, const QString& description) :
    ErdChange(description, true), db(db), tableName(tableName),
    lastPosition(pos), lastCustomColor(customColor)
{
    tableModifier = new TableModifier(db, tableName);
    tableModifier->dropTable();
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

ErdEffectiveChange ErdChangeDeleteEntity::toEffectiveChange() const
{
    return ErdEffectiveChange::drop(tableName, description);
}

QString ErdChangeDeleteEntity::defaultDescription(const QString& tableName)
{
    return QObject::tr("Drop table \"%1\".", "ERD editor").arg(tableName);
}

QStringList ErdChangeDeleteEntity::getChangeDdl()
{
    return tableModifier->getGeneratedSqls();
}

