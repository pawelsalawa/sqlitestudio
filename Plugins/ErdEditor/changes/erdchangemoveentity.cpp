#include "erdchangemoveentity.h"
#include "common/unused.h"

ErdChangeMoveEntity::ErdChangeMoveEntity(const QString& entityName, const QPointF& fromPos, const QPointF& toPos, const QString& description) :
    ErdChange(Category::LAYOUT, description), entityName(entityName), fromPos(fromPos), toPos(toPos)
{
}

void ErdChangeMoveEntity::apply(ErdScene::SceneChangeApi& api)
{
    // Initial entity movements are already applied when the change is created.
    UNUSED(api);
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

QStringList ErdChangeMoveEntity::getChangeDdl()
{
    return {};
}
