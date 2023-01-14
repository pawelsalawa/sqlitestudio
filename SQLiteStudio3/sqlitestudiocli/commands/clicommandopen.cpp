#include "clicommandopen.h"
#include "cli.h"
#include "services/dbmanager.h"
#include <QFile>
#include <QDir>
#include <QDebug>

void CliCommandOpen::execute()
{
    if (!syntax.isArgumentSet(DB_NAME_OR_FILE) && !cli->getCurrentDb())
    {
        println(tr("Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.")
                .arg(cmdName("open"), cmdName("use"), cmdName("open")));
        return;
    }

    Db* db = nullptr;
    if (syntax.isArgumentSet(DB_NAME_OR_FILE))
    {
        QString arg = syntax.getArgument(DB_NAME_OR_FILE);
        db = DBLIST->getByName(arg);
        if (!db)
        {
            if (QFile::exists(arg))
            {
                QString newName = DbManager::generateDbName(arg);
                if (!DBLIST->addDb(newName, arg, false))
                {
                    println(tr("Could not add database %1 to list.").arg(arg));
                    return;
                }
                db = DBLIST->getByName(arg);
                Q_ASSERT(db != nullptr);
            }
            else
            {
                println(tr("File %1 doesn't exist in %2. Cannot open inexisting database with %3 command. "
                                "To create a new database, use %4 command.").arg(arg).arg(QDir::currentPath())
                        .arg(cmdName("open"), cmdName("add")));
                return;
            }
        }
    }
    else
    {
        db = cli->getCurrentDb();
        if (!db)
        {
            qCritical() << "Default database is not in the list!";
            return ;
        }
    }

    if (!db->open())
    {
        println(db->getErrorText());
        return;
    }

    cli->setCurrentDb(db);
    println(tr("Database %1 has been open and set as the current working database.").arg(db->getName()));
}

QString CliCommandOpen::shortHelp() const
{
    return tr("opens database connection");
}

QString CliCommandOpen::fullHelp() const
{
    return tr(
                "Opens connection to the database. If no additional argument was passed, then the connection is open to the "
                "current default database (see help for %1 for details). However if an argument was passed, it can be either "
                "<name> of the registered database to open, or it can be <path> to the database file to open. "
                "In the second case, the <path> gets registered on the list with a generated name, but only for the period "
                "of current application session. After restarting application such database is not restored on the list."
             ).arg(cmdName("use"));
}

void CliCommandOpen::defineSyntax()
{
    syntax.setName("open");
    syntax.addAlternatedArgument(DB_NAME_OR_FILE, {tr("name", "CLI command syntax"), tr("path", "CLI command syntax")}, false);
}
