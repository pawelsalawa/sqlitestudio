#include "clicommandadd.h"
#include "cli.h"
#include "db/dbmanager.h"

CliCommandAdd *CliCommandAdd::create()
{
    return new CliCommandAdd();
}

bool CliCommandAdd::execute(QStringList args)
{
    if (!dbManager->addDb(args[0], args[1]))
    {
        println(QString("Could not add database %1 to list.").arg(args[1]));
        return false;
    }

    cli->setCurrentDb(dbManager->getByName(args[0]));

    return false;
}

bool CliCommandAdd::validate(QStringList args)
{
    if (args.size() != 2)
    {
        println(".add <db name> <file path>");
        return false;
    }
    return true;
}
