#include "erdchangedeleteentity.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "tablemodifier.h"

ErdChangeDeleteEntity::ErdChangeDeleteEntity(Db* db, const QString& tableName, const QString& description) :
    ErdChange(Category::ENTITY_DELETE, description, true), db(db), tableName(tableName)
{
}

ErdChangeDeleteEntity::~ErdChangeDeleteEntity()
{
    safe_delete(tableModifier);
}

QStringList ErdChangeDeleteEntity::getChangeDdl()
{
    if (!tableModifier)
    {
        tableModifier = new TableModifier(db, tableName);
        tableModifier->dropTable();
    }
    return tableModifier->generateSqls();
}

QString ErdChangeDeleteEntity::getTableName() const
{
    return tableName;
}

TableModifier* ErdChangeDeleteEntity::getTableModifier() const
{
    return tableModifier;
}
