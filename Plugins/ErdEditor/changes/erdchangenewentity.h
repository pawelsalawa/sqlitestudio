#ifndef ERDCHANGENEWENTITY_H
#define ERDCHANGENEWENTITY_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QPointF>

class ErdEntity;
class Db;

class ErdChangeNewEntity : public ErdChange
{
    public:
        ErdChangeNewEntity(Db* db, const QString& temporaryEntityName, const SqliteCreateTablePtr& createTable, const QString& description);

        QString getTableName() const;
        QString getTemporaryEntityName() const;

        QPointF getLastPositionBeforeUndo() const;
        void setLastPositionBeforeUndo(QPointF pos);

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        QString temporaryEntityName;
        SqliteCreateTablePtr createTable;
        QPointF lastPositionBeforeUndo;
};

#endif // ERDCHANGENEWENTITY_H
