#include "clicommandclose.h"
#include "cli.h"
#include "db/db.h"
#include "db/dbmanager.h"

bool CliCommandClose::execute(QStringList args)
{
    if (args.size() == 1)
    {
        Db* db = dbManager->getByName(args[0]);
        if (db)
        {
            db->close();
            println(tr("Connection to database %1 closed.").arg(db->getName()));
        }
        else
            println(tr("Database %s is unknown. Use .dblist to see list of known databases.").arg(args[0]));
    }
    else if (cli->getCurrentDb())
    {
        cli->getCurrentDb()->close();
        println(tr("Connection to database %1 closed.").arg(cli->getCurrentDb()->getName()));
    }

    return false;
}

bool CliCommandClose::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    if (args.size() == 0 && !cli->getCurrentDb())
    {
        println(tr("Cannot call .close when no database is set to be current. Specify current database with .use command or pass database name to .close."));
        return false;
    }

    return true;
}

QString CliCommandClose::shortHelp() const
{
    return tr("closes given (or current) database");
}

QString CliCommandClose::fullHelp() const
{
    return tr(
                "Closes database connection. If the database was already closed, nothing happens. "
                "If <name> is provided, it should be name of the database to close (as printed by .dblist command). "
                "The the <name> is not provided, then current working database is closed (see help for .use for details). "
             );
}

QString CliCommandClose::usage() const
{
    return tr("close [<name>]");
}
