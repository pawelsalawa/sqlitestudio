#ifndef ERDCHANGEMOVEENTITY_H
#define ERDCHANGEMOVEENTITY_H

#include "erdchange.h"

class ErdChangeMoveEntity : public ErdChange
{
    public:
        ErdChangeMoveEntity(const QString& entityName, const QPointF& fromPos, const QPointF& toPos, const QString& description);

        QStringList toDdl();

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);
        void applyRedo(ErdScene::SceneChangeApi& api);

    protected:
        QStringList getChangeDdl();

    private:
        QString entityName;
        QPointF fromPos;
        QPointF toPos;
};

#endif // ERDCHANGEMOVEENTITY_H
