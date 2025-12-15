#include "erdchangeentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "tablemodifier.h"
#include "erdentity.h"

ErdChangeEntity::ErdChangeEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after) :
    ErdChange(Category::ENTITY_CHANGE, true), db(db), before(before), after(after)
{
}

ErdChangeEntity::~ErdChangeEntity()
{
    safe_delete(tableModifier);
}

QStringList ErdChangeEntity::getChangeDdl()
{
    after->rebuildTokens();
    tableModifier = new TableModifier(db, before->database, before->table);
    // TODO defer data copying, retain original table with data as renamed
    // Coming back to above after some time - it seems that it's no longer relevant, since ERD uses memDb without data in it for live-editing.
    // This comment should be implemented or removed when ERD plugin is complete.
    tableModifier->alterTable(after);
    return tableModifier->generateSqls();
}

TableModifier* ErdChangeEntity::getTableModifier() const
{
    return tableModifier;
}

QString ErdChangeEntity::getTableNameBefore() const
{
    return before->table;
}

QString ErdChangeEntity::getTableNameAfter() const
{
    return after->table;
}
