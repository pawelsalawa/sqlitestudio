#ifndef ERDCHANGEDELETEENTITY_H
#define ERDCHANGEDELETEENTITY_H

#include "erdchange.h"
#include <QPointF>

class TableModifier;
class Db;

class ErdChangeDeleteEntity : public ErdChange
{
    public:
        ErdChangeDeleteEntity(Db* db, const QString& tableName, const QPointF& pos, const QString& description);
        ~ErdChangeDeleteEntity();

        QString getTableName() const;
        TableModifier *getTableModifier() const;
        QPointF getLastPosition() const;

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        QString tableName;
        TableModifier* tableModifier = nullptr;
        QPointF lastPosition;
};

#endif // ERDCHANGEDELETEENTITY_H
