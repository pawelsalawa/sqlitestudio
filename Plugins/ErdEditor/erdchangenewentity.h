#ifndef ERDCHANGENEWENTITY_H
#define ERDCHANGENEWENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class ErdEntity;
class Db;

class ErdChangeNewEntity : public ErdChange
{
    public:
        ErdChangeNewEntity(Db* db, const SqliteCreateTablePtr& createTable);

        QString getTableName() const;

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        SqliteCreateTablePtr createTable;
};

#endif // ERDCHANGENEWENTITY_H
