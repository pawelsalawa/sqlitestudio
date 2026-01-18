#ifndef ERDCHANGEMODIFYENTITY_H
#define ERDCHANGEMODIFYENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class Db;
class TableModifier;
class ErdEntity;

class ErdChangeModifyEntity : public ErdChange
{
    public:
        ErdChangeModifyEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description);
        ~ErdChangeModifyEntity();

        void apply(ErdScene::SceneChangeApi& api);
        void applyUndo(ErdScene::SceneChangeApi& api);
        ErdEffectiveChange toEffectiveChange() const;

        static QString defaultDescription(const QString& tableName);

    protected:
        QStringList getChangeDdl();

    private:
        void executeChange(ErdScene::SceneChangeApi& api, bool forwardExecution);

        Db* db = nullptr;
        SqliteCreateTablePtr beforeCreateTable;
        SqliteCreateTablePtr afterCreateTable;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEMODIFYENTITY_H
