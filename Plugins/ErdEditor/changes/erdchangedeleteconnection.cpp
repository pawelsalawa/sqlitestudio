#include "erdchangedeleteconnection.h"
#include "scene/erdconnection.h"
#include "tablemodifier.h"
#include "scene/erdentity.h"

ErdChangeDeleteConnection::ErdChangeDeleteConnection(Db* db, ErdConnection* connection, const QString& description) :
    ErdChange(description, true), db(db)
{
    createTable = connection->getStartEntity()->getTableModel();
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
}

ErdChangeDeleteConnection::~ErdChangeDeleteConnection()
{
    safe_delete(tableModifier);
}

void ErdChangeDeleteConnection::apply(ErdScene::SceneChangeApi& api)
{
    QStringList tables = tableModifier->getModifiedTables();
    tables << createTable->table;
    api.refreshEntitiesByTableNames(tables);
}

void ErdChangeDeleteConnection::applyUndo(ErdScene::SceneChangeApi& api)
{
    apply(api); // currently exactly the same as apply
}

QStringList ErdChangeDeleteConnection::getChangeDdl()
{
    if (!tableModifier)
    {
        tableModifier = new TableModifier(db, createTable->table);
        tableModifier->removeFk(endEntityName, columnPairs);
    }
    return tableModifier->generateSqls();
}
