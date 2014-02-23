#include "clicommanddblist.h"
#include "cli.h"
#include "db/dbmanager.h"
#include "unused.h"

#include <QList>

CliCommandDbList *CliCommandDbList::create()
{
    return new CliCommandDbList();
}

void CliCommandDbList::execute(QStringList args)
{
    UNUSED(args);
    QString currentName;
    if (cli->getCurrentDb())
        currentName = cli->getCurrentDb()->getName();

    println("Databases:");
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
            msg += " <- Current";
        }
        println(msg.arg(db->getName()).arg(path).arg(open ? "Open" : "Closed"));
    }
}

bool CliCommandDbList::validate(QStringList args)
{
    UNUSED(args);
    return true;
}
