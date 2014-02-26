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
            println("No current database selected.");
            return false;
        }
        println("Current database:");
        println(cli->getCurrentDb()->getName());
        return false;
    }

    Db* db = dbManager->getByName(args[0]);
    if (!db)
    {
        println("No such database: "+args[0]);
        return false;
    }

    cli->setCurrentDb(db);
    CLI_CFG.General.DefaultDatabase.set(db->getName());

    println("Current database:");
    println(db->getName());

    return false;
}

bool CliCommandUse::validate(QStringList args)
{
    if (args.size() > 1)
    {
        println("Usage: .use [<db name>]");
        return false;
    }
    return true;
}



