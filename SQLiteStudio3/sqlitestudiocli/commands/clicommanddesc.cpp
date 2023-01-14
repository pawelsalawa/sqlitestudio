#include "clicommanddesc.h"
#include "cli.h"
#include "schemaresolver.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "cliutils.h"

CliCommandDesc::CliCommandDesc()
{
}

void CliCommandDesc::execute()
{
    if (!cli->getCurrentDb())
    {
        println(tr("No working database is set.\n"
                   "Call %1 command to set working database.\n"
                   "Call %2 to see list of all databases.")
                .arg(cmdName("use"), cmdName("dblist")));

        return;
    }

    Db* db = cli->getCurrentDb();
    if (!db || !db->isOpen())
    {
        println(tr("Database is not open."));
        return;
    }

    QString table = syntax.getArgument(TABLE);
    SchemaResolver resolver(db);
    SqliteQueryPtr query = resolver.getParsedObject(table, SchemaResolver::TABLE);
    if (!query || (!query.dynamicCast<SqliteCreateTable>() && !query.dynamicCast<SqliteCreateVirtualTable>()))
    {
        println(tr("Cannot find table named: %1").arg(table));
        return;
    }

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (createTable)
    {
        printTable(createTable.data());
        return;
    }

    SqliteCreateVirtualTablePtr virtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    printVirtualTable(virtualTable.data());
}

QString CliCommandDesc::shortHelp() const
{
    return tr("shows details about the table");
}

QString CliCommandDesc::fullHelp() const
{
    return shortHelp();
}

void CliCommandDesc::defineSyntax()
{
    syntax.setName("desc");
    syntax.addArgument(TABLE, tr("table"));
}

void CliCommandDesc::printTable(SqliteCreateTable *table)
{
    int termCols = getCliColumns();
    println(pad("", termCols, '-'));
    println(tr("Table: %1").arg(table->table));

    // Header
    QString msg;
    msg = pad(tr("Column name"), 20, ' ');
    msg += "|";
    msg += pad(tr("Data type"), 10, ' ');
    msg += "|";
    int lgt3rd = termCols - msg.length();
    printHorizontalLine(lgt3rd);
    msg += pad(tr("Constraints"), lgt3rd, ' ');
    println(msg);
    printHorizontalLine(lgt3rd);

    // Rows
    QString constrJoinStr = "\n" + pad("", 20, ' ') + "|" + pad("", 10, ' ') + "|";
    QStringList constrList;
    for (SqliteCreateTable::Column*& column : table->columns)
    {
        msg = pad(column->name.left(20), 20, ' ');
        msg += "|";
        msg += pad((column->type ? column->type->detokenize().left(10) : ""), 10, ' ');
        msg += "|";

        constrList.clear();
        for (SqliteCreateTable::Column::Constraint* constr : column->constraints)
            constrList << pad(constr->detokenize().left(lgt3rd), lgt3rd, ' ');

        msg += constrList.join(constrJoinStr);
        println(msg);
    }
}

void CliCommandDesc::printVirtualTable(SqliteCreateVirtualTable *table)
{
    println(tr("Virtual table: %1").arg(table->table));
    if (table->args.size() > 0)
    {
        int i = 1;
        println(tr("Construction arguments:"));
        for (const QString& arg : table->args)
            println(pad(QString::number(i++), 2, ' ') + ". " + arg);
    }
    else
        println(tr("No construction arguments were passed for this virtual table."));
}

void CliCommandDesc::printHorizontalLine(int lgt3rd)
{
    println(pad("", 20, '-') + "+" + pad("", 10, '-') + "+" + pad("", lgt3rd, '-'));
}
