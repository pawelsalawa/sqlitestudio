#include "clicommanduse.h"
#include "cli.h"
#include "sqlitestudio.h"
#include "config.h"
#include "../cli_config.h"
#include "db/dbmanager.h"

bool CliCommandUse::execute(QStringList args)
{
    if (args.size() == 0)
    {
        if (!cli->getCurrentDb())
        {
            println(tr("No current database selected."));
            return false;
        }
        println(tr("Current database: %1").arg(cli->getCurrentDb()->getName()));
        return false;
    }

    Db* db = dbManager->getByName(args[0]);
    if (!db)
    {
        println(tr("No such database: %1").arg(args[0]));
        return false;
    }

    cli->setCurrentDb(db);
    CFG_CLI.Console.DefaultDatabase.set(db->getName());

    println(tr("Current database: %1").arg(db->getName()));

    return false;
}

bool CliCommandUse::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }
    return true;
}

QString CliCommandUse::shortHelp() const
{
    return tr("changes default working database");
}

QString CliCommandUse::fullHelp() const
{
    return tr(
                "Changes current working database to <name>. If the <name> database is not registered in the application, "
                "then the error message is printed and no change is made.\n"
                "\n"
                "What is current working database?\n"
                "When you type a SQL query to be executed, it is executed on the default database, which is also known as "
                "the current working database. Most of database-related commands can also work using default database, if no database was "
                "provided in their arguments. The current database is always identified by command line prompt. "
                "The default database is always defined (unless there is no database on the list at all).\n"
                "\n"
                "The default database can be selected in various ways:\n"
                "- using %1 command,\n"
                "- by passing database file name to the application startup parameters,\n"
                "- by passing registered database name to the application startup parameters,\n"
                "- by restoring previously selected default database from saved configuration,\n"
                "- or when default database was not selected by any of the above, then first database from the registered databases list "
                "becomes the default one."
             ).arg(cmdName("use"));
}

QString CliCommandUse::usage() const
{
    return "use "+tr("<name>");
}
