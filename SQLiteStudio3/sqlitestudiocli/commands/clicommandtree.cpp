#include "clicommandtree.h"
#include "cli.h"
#include "common/sortedhash.h"
#include "common/unused.h"

const QString CliCommandTree::metaNodeNameTemplate = "<%1>";

void CliCommandTree::execute()
{
    if (!cli->getCurrentDb())
    {
        println(tr("No current working database is selected. Use %1 to define one and then run %2.").arg(cmdName("use")).arg(cmdName("tree")));
        return;
    }

    bool printColumns = syntax.isOptionSet(COLUMNS);
    bool printSystemObjects = syntax.isOptionSet(SYSTEM_OBJECTS);

    SchemaResolver resolver(cli->getCurrentDb());
    resolver.setIgnoreSystemObjects(!printSystemObjects);

    QStringList databases;
    if (syntax.isArgumentSet(INTRNAL_DB))
    {
        databases << syntax.getArgument(INTRNAL_DB);
    }
    else
    {
        databases << "main" << "temp";
        databases += resolver.getDatabases().toList();
    }

    AsciiTree tree;
    tree.label = cli->getCurrentDb()->getName();
    foreach (const QString& database, databases)
    {
        tree.childs << getDatabaseTree(database, resolver, printColumns);
    }

    println();
    println(toAsciiTree(tree));
    println();
}

AsciiTree CliCommandTree::getDatabaseTree(const QString& database, SchemaResolver& resolver, bool printColumns)
{
    QStringList tables = resolver.getTables(database);
    QStringList views = resolver.getViews(database);

    AsciiTree tree;
    AsciiTree tablesTree;
    AsciiTree viewsTree;

    tablesTree.label = metaNodeNameTemplate.arg(tr("Tables"));
    foreach (const QString& table, tables)
        tablesTree.childs << getTableTree(database, table, resolver, printColumns);

    viewsTree.label = metaNodeNameTemplate.arg(tr("Views"));
    foreach (const QString& view, views)
        viewsTree.childs << getViewTree(database, view, resolver);

    tree.label = database;
    tree.childs << tablesTree << viewsTree;
    return tree;
}

AsciiTree CliCommandTree::getTableTree(const QString& database, const QString& table, SchemaResolver& resolver, bool printColumns)
{
    QStringList columns;
    if (printColumns)
        columns = resolver.getTableColumns(database, table);

    QStringList indexes = resolver.getIndexesForTable(database, table);
    QStringList triggers = resolver.getTriggersForTable(database, table);

    AsciiTree tree;
    AsciiTree columnsTree;
    AsciiTree indexesTree;
    AsciiTree triggersTree;

    if (printColumns)
    {
        columnsTree.label = metaNodeNameTemplate.arg(tr("Columns"));
        foreach (const QString& column, columns)
            columnsTree.childs << getTreeLeaf(column);
    }

    indexesTree.label = metaNodeNameTemplate.arg(tr("Indexes"));
    foreach (const QString& index, indexes)
        indexesTree.childs << getTreeLeaf(index);

    triggersTree.label = metaNodeNameTemplate.arg(tr("Triggers"));
    foreach (const QString& trig, triggers)
        triggersTree.childs << getTreeLeaf(trig);

    if (printColumns)
        tree.childs << columnsTree;

    tree.label = table;
    tree.childs << indexesTree;
    tree.childs << triggersTree;

    return tree;
}

AsciiTree CliCommandTree::getViewTree(const QString& database, const QString& view, SchemaResolver& resolver)
{
    QStringList triggers = resolver.getTriggersForView(database, view);

    AsciiTree tree;
    AsciiTree triggersTree;

    triggersTree.label = metaNodeNameTemplate.arg(tr("Triggers"));
    foreach (const QString& trig, triggers)
        triggersTree.childs << getTreeLeaf(trig);

    tree.label = view;
    tree.childs << triggersTree;

    return tree;
}

AsciiTree CliCommandTree::getTreeLeaf(const QString& column)
{
    AsciiTree tree;
    tree.label = column;
    return tree;
}

QString CliCommandTree::shortHelp() const
{
    return tr("prints all objects in the database as a tree");
}

QString CliCommandTree::fullHelp() const
{
    return tr(
                "Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. "
                "The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.\n"
                "When -c option is given, then also columns will be listed under each table.\n"
                "When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).\n"
                "The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, "
                "but instead it's an internal SQLite database name, like 'main', 'temp', or any attached database name. To print tree for other "
                "registered database, call %1 first to switch the working database, and then use %2 command."
             ).arg(cmdName("use")).arg(cmdName("tree"));
}

void CliCommandTree::defineSyntax()
{
    syntax.setName("tree");
    syntax.addOptionShort(COLUMNS, "c");
    syntax.addOptionShort(SYSTEM_OBJECTS, "s");
    syntax.addArgument(INTRNAL_DB, "database", false);
}
