#ifndef ERDCHANGEENTITY_H
#define ERDCHANGEENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class Db;
class TableModifier;
class ErdEntity;

class ErdChangeEntity : public ErdChange
{
    public:
        ErdChangeEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description);
        ~ErdChangeEntity();

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);

    protected:
        QStringList getChangeDdl();

    private:
        void executeChange(ErdScene::SceneChangeApi& api, bool forwardExecution);

        Db* db = nullptr;
        SqliteCreateTablePtr before;
        SqliteCreateTablePtr after;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEENTITY_H
