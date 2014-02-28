#include "clicommandtables.h"
#include "cli.h"
#include "schemaresolver.h"
#include "db/dbmanager.h"
#include "utils.h"

bool CliCommandTables::execute(QStringList args)
{
    Db* db = nullptr;
    if (args.size() > 0)
    {
        db = dbManager->getByName(args[0]);
        if (!db)
        {
            println(tr("No such database: %1. Use .dblist to see list of known databases.").arg(args[0]));
            return false;
        }
    }
    else if (cli->getCurrentDb())
        db = cli->getCurrentDb();
    else
        return false; // should not happen, cause it was checked in validate()

    if (!db->isOpen())
    {
        println(tr("Database %1 is closed.").arg(db->getName()));
        return false;
    }

    println();

    SchemaResolver resolver(db);
    QSet<QString> dbList;
    dbList << "main" << "temp";
    dbList += resolver.getDatabases();

    int width = longest(dbList.toList()).length();
    width = qMax(width, tr("Database").length());

    println(pad(tr("Database"), width, ' ') + " " + tr("Table"));
    println(pad("", width, '-') + "-------------------");

    foreach (const QString& dbName, dbList)
    {
        foreach (const QString& table, resolver.getTables(dbName))
            println(pad(dbName, width, ' ') + " " + table);
    }

    println();

    return false;
}

bool CliCommandTables::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    if (args.size() == 0 && !cli->getCurrentDb())
    {
        println(tr("Cannot call .tables when no database is set to be current. Specify current database with .use command or pass database name to .tables."));
        return false;
    }

    return true;
}

QString CliCommandTables::shortHelp() const
{
    return tr("prints list of tables in the database");
}

QString CliCommandTables::fullHelp() const
{
    return tr(
                "Prints list of tables in given <database> or in the current working database. "
                "Note, that the <database> should be the name of the registered database (see .dblist). "
                "The output list includes all tables from any other databases attached to the queried database."
             );
}

QString CliCommandTables::usage() const
{
    return "tables "+tr("[<database>]");
}
