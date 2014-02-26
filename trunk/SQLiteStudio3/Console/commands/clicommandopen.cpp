#include "clicommandopen.h"
#include "cli.h"
#include "db/dbmanager.h"
#include <QDebug>

bool CliCommandOpen::execute(QStringList args)
{
    Db* db = nullptr;
    if (args.size() == 1)
    {
        db = dbManager->getByName(args[0]);
        if (!db)
        {
            QString newName = DbManager::generateDbName(args[0]);
            if (!dbManager->addDb(newName, args[0], false))
            {
                println(QString("Could not add database %1 to list.").arg(args[1]));
                return false;
            }
            db = dbManager->getByName(args[0]);
            Q_ASSERT(db != nullptr);
        }
    }
    else
    {
        db = cli->getCurrentDb();
        if (!db)
        {
            qCritical() << "Default database is not in the list!";
            return false;
        }
    }

    if (!db->open())
    {
        println(db->getErrorText());
        return false;
    }

    cli->setCurrentDb(db);
    return false;
}

bool CliCommandOpen::validate(QStringList args)
{
    if (args.size() > 1)
    {
        println("Usage: .open [<db name or file path>]");
        return false;
    }

    if (args.size() == 0 && !cli->getCurrentDb())
    {
        println("Cannot call .open when no database is set to be current. Specify current database with .use command.");
        return false;
    }

    return true;
}
