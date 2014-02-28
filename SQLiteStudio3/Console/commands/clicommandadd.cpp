#include "clicommandadd.h"
#include "cli.h"
#include "db/dbmanager.h"

bool CliCommandAdd::execute(QStringList args)
{
    if (!dbManager->addDb(args[0], args[1]))
    {
        println(tr("Could not add database %1 to list.").arg(args[1]));
        return false;
    }

    cli->setCurrentDb(dbManager->getByName(args[0]));
    println(tr("Database added: %1").arg(cli->getCurrentDb()->getName()));

    return false;
}

bool CliCommandAdd::validate(QStringList args)
{
    if (args.size() != 2)
    {
        printUsage();
        return false;
    }
    return true;
}

QString CliCommandAdd::shortHelp() const
{
    return tr("adds new database to the list");
}

QString CliCommandAdd::fullHelp() const
{
    return tr(
                "Adds given database pointed by <path> with given <name> to list the databases list. "
                "The <name> is just a symbolic name that you can later refer to. Just pick any unique name. "
                "For list of databases already on the list use .dblist command."
             );
}

QString CliCommandAdd::usage() const
{
    return "add "+tr("<name> <path>");
}
