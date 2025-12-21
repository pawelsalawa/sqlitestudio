#ifndef ERDCHANGEENTITY_H
#define ERDCHANGEENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class Db;
class TableModifier;
class ErdEntity;

class ErdChangeEntity : public ErdChange
{
    public:
        ErdChangeEntity(Db* db, const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description);
        ~ErdChangeEntity();

        TableModifier *getTableModifier() const;
        QString getTableNameBefore() const;
        QString getTableNameAfter() const;

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        SqliteCreateTablePtr before;
        SqliteCreateTablePtr after;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEENTITY_H
