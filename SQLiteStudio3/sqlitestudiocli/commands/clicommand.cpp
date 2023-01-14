#include "clicommand.h"
#include "qio.h"
#include "cli_config.h"
#include "cliutils.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "clicommandfactory.h"
#include "services/dbmanager.h"
#include "schemaresolver.h"
#include "cli.h"

#include <QDir>

CliCommand::CliCommand()
{
}

CliCommand::~CliCommand()
{
}

void CliCommand::setup(CLI *cli)
{
    this->cli = cli;
    defineSyntax();
}

bool CliCommand::isAsyncExecution() const
{
    return false;
}

bool CliCommand::parseArgs(const QStringList& args)
{
    bool res = syntax.parse(args);

    if (!res && !syntax.getErrorText().isEmpty())
    {
        println(syntax.getErrorText());
        println();
    }

    return res;
}

QString CliCommand::usage() const
{
    return syntax.getSyntaxDefinition();
}

QString CliCommand::usage(const QString& alias) const
{
    return syntax.getSyntaxDefinition(alias);
}

QString CliCommand::getName() const
{
    return syntax.getName();
}

QStringList CliCommand::complete(const QStringList& args)
{
    syntax.parse(args.mid(0, args.size() - 1));

    QStringList results;
    results += syntax.getStrictArgumentCandidates();
    for (int id : syntax.getRegularArgumentCandidates())
        results += getCompletionValuesFor(id, args.last());

    return results;
}

QStringList CliCommand::aliases() const
{
    return syntax.getAliases();
}

void CliCommand::println(const QString &str)
{
    qOut << str << "\n";
    qOut.flush();
}

void CliCommand::print(const QString& str)
{
    qOut << str;
    qOut.flush();
}

void CliCommand::printBox(const QString& str)
{
    int cols = getCliColumns();

    QStringList lines = str.split("\n");
    println(".---------------------------");
    for (const QString& line : lines)
    {
        for (const QString& lineWithMargin : applyMargin(line, cols - 3)) // 2 for "| " and 1 for final new line character
            println("| " + lineWithMargin);
    }

    println("`---------------------------");
}

void CliCommand::printUsage()
{
    println(tr("Usage: %1%2").arg(CFG_CLI.Console.CommandPrefixChar.get(), usage()));
    println("");
}

QString CliCommand::getFilterAndFixDir(QDir& dir, const QString& path)
{
    if (path.isEmpty())
        return "*";

    QString filter;
    QDir tempDir;
    tempDir.setPath(path);
    if (tempDir.exists() && path.endsWith("/"))
    {
        dir.cd(path);
        filter = "*";
    }
    else if (tempDir.cdUp())
    {
        dir.setPath(path);
        dir.cdUp();
        filter = QFileInfo(path).fileName() + "*";
    }
    else
    {
        filter = path;
    }
    return filter;
}

QStringList CliCommand::getCompletionDbNames()
{
    QStringList results = DBLIST->getDbNames();
    results.sort(Qt::CaseInsensitive);
    return results;
}

QStringList CliCommand::getCompletionTables()
{
    QStringList results;
    if (!cli->getCurrentDb())
        return results;

    SchemaResolver resolver(cli->getCurrentDb());
    resolver.setIgnoreSystemObjects(true);
    results += wrapObjNamesIfNeeded(resolver.getTables());
    results += prefixEach("temp.", wrapObjNamesIfNeeded(resolver.getTables("temp")));
    for (const QString& database : resolver.getDatabases())
        results += prefixEach(wrapObjIfNeeded(database)+".", wrapObjNamesIfNeeded(resolver.getTables(database)));

    return results;
}

QStringList CliCommand::getCompletionIndexes()
{
    QStringList results;
    if (!cli->getCurrentDb())
        return results;

    SchemaResolver resolver(cli->getCurrentDb());
    resolver.setIgnoreSystemObjects(true);
    results += wrapObjNamesIfNeeded(resolver.getIndexes());
    results += prefixEach("temp.", wrapObjNamesIfNeeded(resolver.getIndexes("temp")));
    for (const QString& database : resolver.getDatabases())
        results += prefixEach(wrapObjIfNeeded(database)+".", wrapObjNamesIfNeeded(resolver.getIndexes(database)));

    return results;
}

QStringList CliCommand::getCompletionTriggers()
{
    QStringList results;
    if (!cli->getCurrentDb())
        return results;

    SchemaResolver resolver(cli->getCurrentDb());
    resolver.setIgnoreSystemObjects(true);
    results += wrapObjNamesIfNeeded(resolver.getTriggers());
    results += prefixEach("temp.", wrapObjNamesIfNeeded(resolver.getTriggers("temp")));
    for (const QString& database : resolver.getDatabases())
        results += prefixEach(wrapObjIfNeeded(database)+".", wrapObjNamesIfNeeded(resolver.getTriggers(database)));

    return results;
}

QStringList CliCommand::getCompletionViews()
{
    QStringList results;
    if (!cli->getCurrentDb())
        return results;

    SchemaResolver resolver(cli->getCurrentDb());
    resolver.setIgnoreSystemObjects(true);
    results += wrapObjNamesIfNeeded(resolver.getViews());
    results += prefixEach("temp.", wrapObjNamesIfNeeded(resolver.getViews("temp")));
    for (const QString& database : resolver.getDatabases())
        results += prefixEach(wrapObjIfNeeded(database)+".", wrapObjNamesIfNeeded(resolver.getViews(database)));

    return results;
}

QStringList CliCommand::getCompletionDbNamesOrFiles(const QString& partialValue)
{
    QStringList results = getCompletionDbNames();
    results += getCompletionFiles(partialValue);
    return results;
}

QStringList CliCommand::getCompletionFiles(const QString& partialValue)
{
    QDir dir;
    QString filter = getFilterAndFixDir(dir, partialValue);
    QFileInfoList entries = dir.entryInfoList({filter}, QDir::Files, QDir::LocaleAware|QDir::Name);

    QStringList results;
    QString name;
    for (const QFileInfo& entry : entries)
    {
        name = entry.fileName();
        if (dir != QDir::current())
            name.prepend(dir.path() + "/");

        results << name;
    }

    return results;
}

QStringList CliCommand::getCompletionDirs(const QString& partialValue)
{
    QDir dir;
    QString filter = getFilterAndFixDir(dir, partialValue);
    QFileInfoList entries = dir.entryInfoList({filter}, QDir::Dirs|QDir::NoDotAndDotDot, QDir::LocaleAware|QDir::Name);

    QStringList results;
    QString name;
    for (const QFileInfo& entry : entries)
    {
        name = entry.fileName();
        if (dir != QDir::current())
            name.prepend(dir.path() + "/");

        results << name;
    }

    return results;
}

QStringList CliCommand::getCompletionDirsOrFiles(const QString& partialValue)
{
    QStringList results = getCompletionDirs(partialValue);
    results += getCompletionFiles(partialValue);
    return results;
}

QStringList CliCommand::getCompletionInternalDbs()
{
    QStringList results;
    if (!cli->getCurrentDb())
        return results;

    SchemaResolver resolver(cli->getCurrentDb());
    results += resolver.getDatabases().values();
    results << "main" << "temp";
    results.sort(Qt::CaseInsensitive);
    return results;
}

QStringList CliCommand::getCompletionValuesFor(int id, const QString& partialValue)
{
    QStringList results;
    if (id < 1000) // this base implementation is only for local enum values (>= 1000).
        return results;

    switch (static_cast<ArgIds>(id))
    {
        case CliCommand::DB_NAME:
            results += getCompletionDbNames();
            break;
        case CliCommand::DB_NAME_OR_FILE:
            results += getCompletionDbNamesOrFiles(partialValue);
            break;
        case CliCommand::FILE_PATH:
            results += getCompletionFiles(partialValue);
            break;
        case CliCommand::DIR_PATH:
            results += getCompletionDirs(partialValue);
            break;
        case CliCommand::CMD_NAME:
            results += CliCommandFactory::getCommandNames();
            break;
        case CliCommand::DIR_OR_FILE:
            results += getCompletionDirsOrFiles(partialValue);
            break;
        case CliCommand::INTRNAL_DB:
            results += getCompletionInternalDbs();
            break;
        case CliCommand::TABLE:
            results += getCompletionTables();
            break;
        case CliCommand::INDEX:
            results += getCompletionIndexes();
            break;
        case CliCommand::TRIGGER:
            results += getCompletionTriggers();
            break;
        case CliCommand::VIEW:
            results += getCompletionViews();
            break;
        case CliCommand::STRING:
            break;
    }

    return results;
}

QString CliCommand::cmdName(const QString& cmd)
{
    return CFG_CLI.Console.CommandPrefixChar.get()+cmd;
}
