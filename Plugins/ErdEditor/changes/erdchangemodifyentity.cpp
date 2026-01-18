#include "erdchangemodifyentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "erdeffectivechange.h"
#include "tablemodifier.h"
#include "scene/erdentity.h"

ErdChangeModifyEntity::ErdChangeModifyEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description) :
    ErdChange(description, true), db(db), beforeCreateTable(before), afterCreateTable(after)
{
    afterCreateTable->rebuildTokens();
    tableModifier = new TableModifier(db, beforeCreateTable->database, beforeCreateTable->table, beforeCreateTable);
    tableModifier->alterTable(afterCreateTable);
}

ErdChangeModifyEntity::~ErdChangeModifyEntity()
{
    safe_delete(tableModifier);
}

QStringList ErdChangeModifyEntity::getChangeDdl()
{
    return tableModifier->getGeneratedSqls();
}

void ErdChangeModifyEntity::executeChange(ErdScene::SceneChangeApi& api, bool forwardExecution)
{
    typedef QPair<QString, QString> OldNewName;
    QList<OldNewName> modifiedTables = {
        forwardExecution ?
        OldNewName(beforeCreateTable->table, afterCreateTable->table) :
        OldNewName(afterCreateTable->table, beforeCreateTable->table)
    };

    for (const QString& tableName : tableModifier->getModifiedTables())
        modifiedTables << OldNewName(tableName, tableName);

    for (OldNewName& oldNewTableName : modifiedTables)
        api.refreshEntity(oldNewTableName.first, oldNewTableName.second);
}

void ErdChangeModifyEntity::apply(ErdScene::SceneChangeApi& api)
{
    executeChange(api, true);
}

void ErdChangeModifyEntity::applyUndo(ErdScene::SceneChangeApi& api)
{
    executeChange(api, false);
}

ErdEffectiveChange ErdChangeModifyEntity::toEffectiveChange() const
{
    return ErdEffectiveChange::modify(beforeCreateTable, afterCreateTable, description);
}

QString ErdChangeModifyEntity::defaultDescription(const QString& tableName)
{
    return QObject::tr("Modify table \"%1\".", "ERD editor").arg(tableName);
}

