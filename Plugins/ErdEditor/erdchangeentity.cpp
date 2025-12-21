#include "erdchangeentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "tablemodifier.h"
#include "erdentity.h"

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
