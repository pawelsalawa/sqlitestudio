#ifndef ERDCHANGECOLORENTITY_H
#define ERDCHANGECOLORENTITY_H

#include "erdchange.h"

class ErdChangeColorEntity : public ErdChange
{
    public:
        ErdChangeColorEntity(const QString& entityName, const QColor& oldColor, const QColor& newColor, const QString& description);

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);
        void applyRedo(ErdScene::SceneChangeApi& api);

    protected:
        QStringList getChangeDdl();

    private:
        QString entityName;
        QColor oldColor;
        QColor newColor;

};

#endif // ERDCHANGECOLORENTITY_H
