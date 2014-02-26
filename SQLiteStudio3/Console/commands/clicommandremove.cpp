#include "clicommandremove.h"
#include "cli.h"
#include "db/dbmanager.h"
#include "db/db.h"

CliCommandRemove *CliCommandRemove::create()
{
    return new CliCommandRemove();
}

bool CliCommandRemove::execute(QStringList args)
{
    Db* db = dbManager->getByName(args[0]);
    if (!db)
    {
        println("No such database.");
        return false;
    }

    dbManager->removeDb(db);

    QList<Db*> dblist = dbManager->getDbList();
    if (dblist.size() > 0)
        cli->setCurrentDb(dblist[0]);
    else
        cli->setCurrentDb(nullptr);

    return false;
}

bool CliCommandRemove::validate(QStringList args)
{
    if (args.size() != 1)
    {
        println("Usage: rem <db name>");
        return false;
    }
    return true;
}
