#include "erdchangeentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "tablemodifier.h"
#include "erdentity.h"

ErdChangeEntity::ErdChangeEntity(ErdEntity* entity, Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after) :
    ErdChange(Category::ENTITY_CHANGE), entity(entity), db(db), before(before), after(after)
{
}

QStringList ErdChangeEntity::toDdl()
{
    after->rebuildTokens();
    tableModifier = new TableModifier(db, before->database, before->table);
    // TODO defer data copying, retain original table with data as renamed
    tableModifier->alterTable(after);
    return tableModifier->generateSqls();
}

TableModifier* ErdChangeEntity::getTableModifier() const
{
    return tableModifier;
}

ErdEntity* ErdChangeEntity::getEntity() const
{
    return entity;
}

QString ErdChangeEntity::getTableNameBefore() const
{
    return before->table;
}

QString ErdChangeEntity::getTableNameAfter() const
{
    return after->table;
}
