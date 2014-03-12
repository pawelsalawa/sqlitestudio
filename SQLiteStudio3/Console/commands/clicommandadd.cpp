#include "clicommandadd.h"
#include "cli.h"
#include "services/dbmanager.h"

void CliCommandAdd::execute()
{
    if (!DBLIST->addDb(syntax.getArgument(DB_NAME), syntax.getArgument(FILE_PATH)))
    {
        println(tr("Could not add database %1 to list.").arg(syntax.getArgument(FILE_PATH)));
        return;
    }

    cli->setCurrentDb(DBLIST->getByName(syntax.getArgument(DB_NAME)));
    println(tr("Database added: %1").arg(cli->getCurrentDb()->getName()));
}

QString CliCommandAdd::shortHelp() const
{
    return tr("adds new database to the list");
}

QString CliCommandAdd::fullHelp() const
{
    return tr(
                "Adds given database pointed by <path> with given <name> to list the databases list. "
                "The <name> is just a symbolic name that you can later refer to. Just pick any unique name. "
                "For list of databases already on the list use %1 command."
             ).arg(cmdName("dblist"));
}

void CliCommandAdd::defineSyntax()
{
    syntax.setName("add");
    syntax.addArgument(DB_NAME, tr("name", "CLI command syntax"));
    syntax.addArgument(FILE_PATH, tr("path", "CLI command syntax"));
}
