#include "clicommanddblist.h"
#include "cli.h"
#include "db/dbmanager.h"
#include "unused.h"
#include <QList>

void CliCommandDbList::execute()
{
    if (!cli->getCurrentDb())
    {
        println(tr("No current working database defined."));
        return;
    }

    QString currentName = cli->getCurrentDb()->getName();

    println(tr("Databases:"));
    QList<Db*> dbList = DBLIST->getDbList();
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

void CliCommandDbList::defineSyntax()
{
    syntax.setName("dblist");
    syntax.addAlias("databases");
}
