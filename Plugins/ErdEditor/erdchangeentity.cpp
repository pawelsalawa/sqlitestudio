#include "erdchangeentity.h"
#include "parser/ast/sqlitecreatetable.h"
#include "db/db.h"
#include "tablemodifier.h"
#include "erdentity.h"

ErdChangeEntity::ErdChangeEntity(ErdEntity* entity, Db* db,
                                 const QSharedPointer<SqliteCreateTable>& before,
                                 const QSharedPointer<SqliteCreateTable>& after) :
    ErdChange(Category::DDL), entity(entity), db(db), before(before), after(after)
{
    existingTable = entity->isExistingTable();
}

QStringList ErdChangeEntity::toDdl()
{
    after->rebuildTokens();
    if (!existingTable)
    {
        return QStringList{after->detokenize()};
    }
    else
    {
        tableModifier = new TableModifier(db, before->database, before->table);
        // TODO defer data copying, retain original table with data as renamed
        tableModifier->alterTable(after);
        return tableModifier->generateSqls();
    }
}
