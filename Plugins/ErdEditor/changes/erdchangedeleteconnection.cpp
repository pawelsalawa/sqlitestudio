#include "erdchangedeleteconnection.h"
#include "scene/erdconnection.h"
#include "erdeffectivechange.h"
#include "tablemodifier.h"
#include "scene/erdentity.h"

ErdChangeDeleteConnection::ErdChangeDeleteConnection(Db* db, ErdConnection* connection, const QString& description) :
    ErdChange(description, true), db(db)
{
    beforeCreateTable = connection->getStartEntity()->getTableModel();
    endEntityName = connection->getEndEntity()->getTableName();

    columnPairs << QPair<QString, QString>(
                connection->getStartEntityColumn()->name,
                connection->getEndEntityColumn()->name
            );

    if (connection->isCompoundConnection())
    {
        for (ErdConnection*& conn : connection->getAssociatedConnections())
        {
            columnPairs << QPair<QString, QString>(
                        conn->getStartEntityColumn()->name,
                        conn->getEndEntityColumn()->name
                    );
        }
    }

    tableModifier = new TableModifier(db, beforeCreateTable->table);
    afterCreateTable = tableModifier->removeFk(endEntityName, columnPairs);
}

ErdChangeDeleteConnection::~ErdChangeDeleteConnection()
{
    safe_delete(tableModifier);
}

void ErdChangeDeleteConnection::apply(ErdScene::SceneChangeApi& api)
{
    QStringList tables = tableModifier->getModifiedTables();
    tables << beforeCreateTable->table;
    api.refreshEntitiesByTableNames(tables);
}

void ErdChangeDeleteConnection::applyUndo(ErdScene::SceneChangeApi& api)
{
    apply(api); // currently exactly the same as apply
}

ErdEffectiveChange ErdChangeDeleteConnection::toEffectiveChange() const
{
    return ErdEffectiveChange::modify(beforeCreateTable, afterCreateTable, description);
}

QStringList ErdChangeDeleteConnection::getChangeDdl()
{
    return tableModifier->getGeneratedSqls();
}
