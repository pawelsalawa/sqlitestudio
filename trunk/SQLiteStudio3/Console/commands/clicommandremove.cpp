#include "clicommandremove.h"
#include "cli.h"
#include "services/dbmanager.h"
#include "db/db.h"

void CliCommandRemove::execute()
{
    QString dbName = syntax.getArgument(DB_NAME);
    Db* db = DBLIST->getByName(dbName);
    if (!db)
    {
        println(tr("No such database: %1").arg(dbName));
        return;
    }

    bool isCurrent = cli->getCurrentDb() == db;
    QString name = db->getName();

    DBLIST->removeDb(db);
    println(tr("Database removed: %1").arg(name));

    QList<Db*> dblist = DBLIST->getDbList();
    if (isCurrent && dblist.size() > 0)
    {
        cli->setCurrentDb(dblist[0]);
        println(tr("New current database set:"));
        println(cli->getCurrentDb()->getName());
    }
    else
        cli->setCurrentDb(nullptr);
}

QString CliCommandRemove::shortHelp() const
{
    return tr("removes database from the list");
}

QString CliCommandRemove::fullHelp() const
{
    return tr(
                "Removes <name> database from the list of registered databases. "
                "If the database was not on the list (see %1 command), then error message is printed "
                "and nothing more happens."
                ).arg(cmdName("dblist"));
}

void CliCommandRemove::defineSyntax()
{
    syntax.setName("remove");
    syntax.addArgument(DB_NAME, tr("name", "CLI command syntax"));
}
