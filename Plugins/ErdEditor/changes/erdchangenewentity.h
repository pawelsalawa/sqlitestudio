#ifndef ERDCHANGENEWENTITY_H
#define ERDCHANGENEWENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QPointF>

class ErdEntity;
class Db;

class ErdChangeNewEntity : public ErdChange
{
    public:
        ErdChangeNewEntity(Db* db, const QString& temporaryEntityName, const SqliteCreateTablePtr& createTable, const QPointF& initialPos, const QString& description);

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);
        void applyRedo(ErdScene::SceneChangeApi& api);
        ErdEffectiveChange toEffectiveChange() const;

        static QString defaultDescription(const QString& tableName);

    protected:
        QStringList getChangeDdl();

    private:
        void refreshReferencingTables(ErdScene::SceneChangeApi& api);

        Db* db = nullptr;
        QString temporaryEntityName;
        SqliteCreateTablePtr createTable;
        QPointF lastPositionBeforeUndo;
};

#endif // ERDCHANGENEWENTITY_H
