#include "clicommanddblist.h"
#include "cli.h"
#include "db/dbmanager.h"
#include "unused.h"
#include <QList>

bool CliCommandDbList::execute(QStringList args)
{
    UNUSED(args);
    QString currentName;
    if (cli->getCurrentDb())
        currentName = cli->getCurrentDb()->getName();

    println(tr("Databases:"));
    QList<Db*> dbList = dbManager->getDbList();
    QString path;
    QString msg;
    QString genericMsg = "%1 (%2) <%3>";
    foreach (Db* db, dbList)
    {
        bool open = db->isOpen();
        path = db->getPath();
        msg = genericMsg;
        if (db->getName() == currentName)
        {
            msg += " <- " + tr("Current");
        }
        println(msg.arg(db->getName()).arg(path).arg(open ? tr("Open") : tr("Closed")));
    }

    return false;
}

bool CliCommandDbList::validate(QStringList args)
{
    UNUSED(args);
    return true;
}

QString CliCommandDbList::shortHelp() const
{
    return tr("prints list of registered databases");
}

QString CliCommandDbList::fullHelp() const
{
    return tr(
                "Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state "
                "and .dblist tells you that. The current working database (aka default database) is also marked on the list. "
                "See help for %1 command to learn about the default database."
                ).arg(cmdName("use"));
}

QString CliCommandDbList::usage() const
{
    return "dblist";
}

QStringList CliCommandDbList::aliases() const
{
    return {"databases"};
}
