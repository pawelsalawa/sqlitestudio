#ifndef ERDCHANGECOMPOSITE_H
#define ERDCHANGECOMPOSITE_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class Db;

class ErdChangeComposite : public ErdChange
{
    public:
        explicit ErdChangeComposite(const QString& description);
        ErdChangeComposite(QList<ErdChange*> changes, const QString& description);
        void addChange(ErdChange* change);
        ErdChangeComposite& operator<<(ErdChange* change);
        ErdChangeComposite& operator+=(const QList<ErdChange*>& changeList);
        QList<ErdChange*> getChanges() const;
        QStringList getUndoDdl();
        QString getTransactionId() const;

    protected:
        QStringList getChangeDdl();

    private:
        QList<ErdChange*> changes;
};

#endif // ERDCHANGECOMPOSITE_H
