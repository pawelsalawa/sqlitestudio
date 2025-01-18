#include "erdchangeentity.h"
#include "db/db.h"
#include "tablemodifier.h"

ErdChangeEntity::ErdChangeEntity(ErdEntity* entity, Db* db, SqliteCreateTablePtr before, SqliteCreateTablePtr after) :
    ErdChange(Category::DDL), entity(entity), db(db)
{
    this->before = before;
    this->after = after;
    tableModifier = new TableModifier(db, before->table);
}

QString ErdChangeEntity::toDdl() const
{
    return QString();
}
