#ifndef ERDCHANGEDELETEENTITY_H
#define ERDCHANGEDELETEENTITY_H

#include "erdchange.h"

class TableModifier;
class Db;

class ErdChangeDeleteEntity : public ErdChange
{
    public:
        ErdChangeDeleteEntity(Db* db, const QString& tableName, const QString& description);
        ~ErdChangeDeleteEntity();

        QString getTableName() const;
        TableModifier *getTableModifier() const;

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        QString tableName;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEDELETEENTITY_H
