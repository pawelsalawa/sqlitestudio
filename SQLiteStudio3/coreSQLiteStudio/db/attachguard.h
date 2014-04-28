#ifndef ATTACHGUARD_H
#define ATTACHGUARD_H

#include <QSharedPointer>

class Db;

class GuardedAttach
{
    public:
        GuardedAttach(Db* db, Db* attachedDb, const QString& attachName);
        virtual ~GuardedAttach();

        QString getName() const;

    private:
        Db* db = nullptr;
        Db* attachedDb = nullptr;
        QString name;
};

typedef QSharedPointer<GuardedAttach> AttachGuard;

#endif // ATTACHGUARD_H
