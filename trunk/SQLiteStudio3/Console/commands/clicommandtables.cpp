#include "clicommandtables.h"
#include "cli.h"
#include "schemaresolver.h"
#include "db/dbmanager.h"
#include "utils.h"

void CliCommandTables::execute(const QStringList& args)
{
    Db* db = nullptr;
    if (args.size() > 0)
    {
        db = DBLIST->getByName(args[0]);
        if (!db)
        {
            println(tr("No such database: %1. Use .dblist to see list of known databases.").arg(args[0]));
            return;
        }
    }
    else if (cli->getCurrentDb())
    {
        db = cli->getCurrentDb();
    }
    else
    {
        println(tr("Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.")
                .arg(cmdName("tables")).arg(cmdName("use")).arg(cmdName("tables")));
        return;
    }

    if (!db->isOpen())
    {
        println(tr("Database %1 is closed.").arg(db->getName()));
        return;
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
}

QString CliCommandTables::shortHelp() const
{
    return tr("prints list of tables in the database");
}

QString CliCommandTables::fullHelp() const
{
    return tr(
                "Prints list of tables in given <database> or in the current working database. "
                "Note, that the <database> should be the name of the registered database (see %1). "
                "The output list includes all tables from any other databases attached to the queried database."
                ).arg(cmdName("use"));
}

void CliCommandTables::defineSyntax()
{
    syntax.setName("tables");
    syntax.addArgument(DB_NAME, tr("database", "CLI command syntax"), false);
}
