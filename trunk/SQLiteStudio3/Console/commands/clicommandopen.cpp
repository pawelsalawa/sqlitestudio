#include "clicommandopen.h"
#include "cli.h"
#include "db/dbmanager.h"
#include <QFile>
#include <QDir>
#include <QDebug>

bool CliCommandOpen::execute(QStringList args)
{
    Db* db = nullptr;
    if (args.size() == 1)
    {
        db = dbManager->getByName(args[0]);
        if (!db)
        {
            if (QFile::exists(args[0]))
            {
                QString newName = DbManager::generateDbName(args[0]);
                if (!dbManager->addDb(newName, args[0], false))
                {
                    println(tr("Could not add database %1 to list.").arg(args[1]));
                    return false;
                }
                db = dbManager->getByName(args[0]);
                Q_ASSERT(db != nullptr);
            }
            else
            {
                println(tr("File %1 doesn't exist in %2. Cannot open inexisting database with .open command. "
                                "To create a new database, use .add command.").arg(args[1]).arg(QDir::currentPath()));
                return false;
            }
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
    println(tr("Database %1 is now open and set to the current working database.").arg(db->getName()));

    return false;
}

bool CliCommandOpen::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    if (args.size() == 0 && !cli->getCurrentDb())
    {
        println(tr("Cannot call .open when no database is set to be current. Specify current database with .use command or pass database name to .open."));
        return false;
    }

    return true;
}

QString CliCommandOpen::shortHelp() const
{
    return tr("opens database connection");
}

QString CliCommandOpen::fullHelp() const
{
    return tr(
                "Opens connection to the database. If no additional argument was passed, then the connection is open to the "
                "current default database (see help for .use for details). However if an argument was passed, it can be either "
                "<name> of the registered database to open, or it can be <path> to the database file to open. "
                "In the second case, the <path> gets registered on the list with a generated name, but only for the period "
                "of current application session. After restarting application such database is not restored on the list."
             );
}

QString CliCommandOpen::usage() const
{
    return "open "+tr("[<name|path>]");
}
