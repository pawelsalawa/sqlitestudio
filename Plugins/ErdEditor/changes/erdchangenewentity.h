#ifndef ERDCHANGENEWENTITY_H
#define ERDCHANGENEWENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class ErdEntity;
class Db;

class ErdChangeNewEntity : public ErdChange
{
    public:
        ErdChangeNewEntity(Db* db, const QString& temporaryEntityName, const SqliteCreateTablePtr& createTable, const QString& description);

        QString getTableName() const;
        QString getTemporaryEntityName() const;

    protected:
        QStringList provideUndoEntitiesToRefresh() const;
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        QString temporaryEntityName;
        SqliteCreateTablePtr createTable;
};

#endif // ERDCHANGENEWENTITY_H
