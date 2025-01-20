#ifndef ERDCHANGEENTITY_H
#define ERDCHANGEENTITY_H

#include "erdchange.h"
#include <QSharedPointer>

class Db;
class TableModifier;
class ErdEntity;
class SqliteCreateTable;

class ErdChangeEntity : public ErdChange
{
    public:
        ErdChangeEntity(ErdEntity* entity, Db* db, const QSharedPointer<SqliteCreateTable>& before, const QSharedPointer<SqliteCreateTable>& after);

        QStringList toDdl();
        TableModifier *getTableModifier() const;
        ErdEntity *getEntity() const;

    private:
        ErdEntity* entity = nullptr;
        Db* db = nullptr;
        QSharedPointer<SqliteCreateTable> before;
        QSharedPointer<SqliteCreateTable> after;
        TableModifier* tableModifier = nullptr;
        bool existingTable = false;
};

#endif // ERDCHANGEENTITY_H
