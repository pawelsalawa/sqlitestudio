#include "erdchangedeleteentity.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "tablemodifier.h"

ErdChangeDeleteEntity::ErdChangeDeleteEntity(Db* db, const QString& tableName, const QPointF& pos, const QString& description) :
    ErdChange(Category::ENTITY_DELETE, description, true), db(db), tableName(tableName), lastPosition(pos)
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

QPointF ErdChangeDeleteEntity::getLastPosition() const
{
    return lastPosition;
}

QString ErdChangeDeleteEntity::getTableName() const
{
    return tableName;
}

TableModifier* ErdChangeDeleteEntity::getTableModifier() const
{
    return tableModifier;
}

QStringList ErdChangeDeleteEntity::provideUndoEntitiesToRefresh() const
{
    QStringList modifiedTables;
    modifiedTables << tableName;
    if (tableModifier)
        modifiedTables += tableModifier->getModifiedTables();

    return modifiedTables;
}
