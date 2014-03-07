#include "clicommandtree.h"

void CliCommandTree::execute(const QStringList& args)
{
    // TODO
}

QString CliCommandTree::shortHelp() const
{
    return tr("prints all objects in the database as a tree");
}

QString CliCommandTree::fullHelp() const
{
    return tr(
                "Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. "
                "The tree is very similar to the one that you can see in GUI client of the SQLiteStudio. "
                "When -c option is given, then also columns will be listed under each table."
             );
}

void CliCommandTree::defineSyntax()
{
    syntax.setName("tree");
    syntax.addOptionShort(COLUMNS, "c");
}
