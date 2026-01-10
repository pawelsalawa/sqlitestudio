#include "erdchangeentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "tablemodifier.h"
#include "scene/erdentity.h"

ErdChangeEntity::ErdChangeEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description) :
    ErdChange(Category::ENTITY_CHANGE, description, true), db(db), before(before), after(after)
{
}

ErdChangeEntity::~ErdChangeEntity()
{
    safe_delete(tableModifier);
}

QStringList ErdChangeEntity::getChangeDdl()
{
    if (!tableModifier)
    {
        after->rebuildTokens();
        tableModifier = new TableModifier(db, before->database, before->table);
        tableModifier->alterTable(after);
    }
    return tableModifier->generateSqls();
}

void ErdChangeEntity::executeChange(ErdScene::SceneChangeApi& api, bool forwardExecution)
{
    typedef QPair<QString, QString> OldNewName;
    QList<OldNewName> modifiedTables = {
        forwardExecution ?
        OldNewName(before->table, after->table) :
        OldNewName(after->table, before->table)
    };

    for (const QString& tableName : tableModifier->getModifiedTables())
        modifiedTables << OldNewName(tableName, tableName);

    for (OldNewName& oldNewTableName : modifiedTables)
        api.refreshEntity(oldNewTableName.first, oldNewTableName.second);
}

void ErdChangeEntity::apply(ErdScene::SceneChangeApi& api)
{
    executeChange(api, true);
}

void ErdChangeEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    executeChange(api, false);
}
