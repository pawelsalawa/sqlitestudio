#include "attachguard.h"
#include "db/db.h"

GuardedAttach::GuardedAttach(Db* db, Db* attachedDb, const QString& attachName) :
    db(db), attachedDb(attachedDb), name(attachName)
{
}

GuardedAttach::~GuardedAttach()
{
    if (name.isNull())
        return;

    db->detach(attachedDb);
}

QString GuardedAttach::getName() const
{
    return name;
}
