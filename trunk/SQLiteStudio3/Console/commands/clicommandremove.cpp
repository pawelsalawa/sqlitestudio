#include "clicommandremove.h"
#include "cli.h"
#include "db/dbmanager.h"
#include "db/db.h"

bool CliCommandRemove::execute(QStringList args)
{
    Db* db = dbManager->getByName(args[0]);
    if (!db)
    {
        println(tr("No such database: %1").arg(args[0]));
        return false;
    }

    bool isCurrent = cli->getCurrentDb() == db;
    QString name = db->getName();

    dbManager->removeDb(db);
    println(tr("Database removed: %1").arg(name));

    QList<Db*> dblist = dbManager->getDbList();
    if (isCurrent && dblist.size() > 0)
    {
        cli->setCurrentDb(dblist[0]);
        println(tr("New current database set:"));
        println(cli->getCurrentDb()->getName());
    }
    else
        cli->setCurrentDb(nullptr);

    return false;
}

bool CliCommandRemove::validate(QStringList args)
{
    if (args.size() != 1)
    {
        printUsage();
        return false;
    }
    return true;
}

QString CliCommandRemove::shortHelp() const
{
    return tr("removes database from the list");
}

QString CliCommandRemove::fullHelp() const
{
    return tr(
                "Removes <name> database from the list of registered databases. "
                "If the database was not on the list (see .dblist command), then error message is printed "
                "and nothing more happens."
             );
}

QString CliCommandRemove::usage() const
{
    return "remove "+tr("<name>");
}
