#include "erdchangedeleteconnection.h"
#include "scene/erdconnection.h"
#include "tablemodifier.h"
#include "scene/erdentity.h"

ErdChangeDeleteConnection::ErdChangeDeleteConnection(Db* db, ErdConnection* connection, const QString& description) :
    ErdChange(Category::CONNECTION_DELETE, description, true), db(db)
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

TableModifier* ErdChangeDeleteConnection::getTableModifier() const
{
    return tableModifier;
}

QString ErdChangeDeleteConnection::getStartEntityName() const
{
    return createTable->table;
}

QStringList ErdChangeDeleteConnection::provideUndoEntitiesToRefresh() const
{
    QStringList modifiedTables;
    if (tableModifier)
        modifiedTables += tableModifier->getModifiedTables();

    modifiedTables << getStartEntityName();
    return modifiedTables;
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
