#include "clicommandclose.h"
#include "cli.h"
#include "db/db.h"
#include "db/dbmanager.h"

CliCommandClose *CliCommandClose::create()
{
    return new CliCommandClose();
}

void CliCommandClose::execute(QStringList args)
{
    if (args.size() == 1)
    {
        Db* db = dbManager->getByName(args[0]);
        if (db)
            db->close();
        else
            println(QString("Database %s is unknown. Use .dblist to see list of known databases.").arg(args[0]));
    }
    else if (cli->getCurrentDb())
        cli->getCurrentDb()->close();
}

bool CliCommandClose::validate(QStringList args)
{
    if (args.size() > 1)
    {
        println("Usage: .close [<db name>]");
        return false;
    }

    if (args.size() == 0 && !cli->getCurrentDb())
    {
        println("Cannot call .close when no database is set to be current. Specify current database with .use command.");
        return false;
    }

    return true;
}
