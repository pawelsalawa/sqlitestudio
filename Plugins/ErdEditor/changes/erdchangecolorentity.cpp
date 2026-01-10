#include "erdchangecolorentity.h"
#include "common/unused.h"

ErdChangeColorEntity::ErdChangeColorEntity(const QString& entityName, const QColor& oldColor, const QColor& newColor, const QString& description) :
    ErdChange(description), entityName(entityName), oldColor(oldColor), newColor(newColor)
{
}

void ErdChangeColorEntity::apply(ErdScene::SceneChangeApi& api)
{
    // Initial entity color is already applied when the change is created.
    UNUSED(api);
}

void ErdChangeColorEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    api.setEntityColor(entityName, oldColor);
    api.updateScene();
}

void ErdChangeColorEntity::applyRedo(ErdScene::SceneChangeApi& api)
{
    api.setEntityColor(entityName, newColor);
    api.updateScene();
}

QStringList ErdChangeColorEntity::getChangeDdl()
{
    return {};
}
