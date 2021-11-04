#include "clicommanddblist.h"
#include "cli.h"
#include "services/dbmanager.h"
#include "common/unused.h"
#include "common/utils.h"
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

    int maxNameLength = tr("Name", "CLI db name column").length();
    int lgt = 0;
    for (Db* db : dbList)
    {
        lgt = db->getName().length() + 1;
        maxNameLength = qMax(maxNameLength, lgt);
    }

    int connStateLength = qMax(tr("Open", "CLI connection state column").length(), tr("Closed", "CLI connection state column").length());
    connStateLength = qMax(connStateLength, tr("Connection", "CLI connection state column").length());

    msg = pad(tr("Name", "CLI db name column"), maxNameLength, ' ');
    msg += "|";
    msg += pad(tr("Connection", "CLI connection state column"), connStateLength, ' ');
    msg += "|";
    msg += tr("Database file path");
    println(msg);

    msg = QString("-").repeated(maxNameLength);
    msg += "+";
    msg += QString("-").repeated(connStateLength);
    msg += "+";
    msg += QString("-").repeated(tr("Database file path").length() + 1);
    println(msg);

    QString name;
    for (Db* db : dbList)
    {
        bool open = db->isOpen();
        path = QDir::toNativeSeparators(db->getPath());
        name = db->getName();
        if (name == currentName)
            name.prepend("*");
        else
            name.prepend(" ");

        msg = pad(name, maxNameLength, ' ');
        msg += "|";
        msg += pad((open ? tr("Open", "CLI connection state column") : tr("Closed", "CLI connection state column")), connStateLength, ' ');
        msg += "|";
        msg += path;
        println(msg);
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
                "and %1 tells you that. The current working database (aka default database) is also marked on the list with '*' at the start of its name. "
                "See help for %2 command to learn about the default database."
                ).arg(cmdName("dblist"), cmdName("use"));
}

void CliCommandDbList::defineSyntax()
{
    syntax.setName("dblist");
    syntax.addAlias("databases");
}
