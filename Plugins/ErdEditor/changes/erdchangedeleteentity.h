#ifndef ERDCHANGEDELETEENTITY_H
#define ERDCHANGEDELETEENTITY_H

#include "erdchange.h"
#include <QColor>
#include <QPointF>

class TableModifier;
class Db;

class ErdChangeDeleteEntity : public ErdChange
{
    public:
        ErdChangeDeleteEntity(Db* db, const QString& tableName, const QPointF& pos, const QColor& customColor, const QString& description);
        ~ErdChangeDeleteEntity();

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);
        ErdEffectiveChange toEffectiveChange() const;

        static QString defaultDescription(const QString& tableName);

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        QString tableName;
        TableModifier* tableModifier = nullptr;
        QPointF lastPosition;
        QColor lastCustomColor;
};

#endif // ERDCHANGEDELETEENTITY_H
