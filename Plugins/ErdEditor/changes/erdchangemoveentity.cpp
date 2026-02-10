#include "erdchangemoveentity.h"

ErdChangeMoveEntity::ErdChangeMoveEntity(const QString& entityName, const QPointF& fromPos, const QPointF& toPos, const QString& description) :
    ErdChange(description), entityName(entityName), fromPos(fromPos), toPos(toPos)
{
}

void ErdChangeMoveEntity::apply(ErdScene::SceneChangeApi& api)
{
    // Initial entity movements are already applied when the change is created.
    Q_UNUSED(api);
}

void ErdChangeMoveEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    api.setEntityPosition(entityName, fromPos);
    api.updateScene();
}

void ErdChangeMoveEntity::applyRedo(ErdScene::SceneChangeApi& api)
{
    api.setEntityPosition(entityName, toPos);
    api.updateScene();
}

QString ErdChangeMoveEntity::defaultDescription(const QString& tableName)
{
    return QObject::tr("Move table \"%1\"").arg(tableName);
}

QStringList ErdChangeMoveEntity::getChangeDdl()
{
    return {};
}
