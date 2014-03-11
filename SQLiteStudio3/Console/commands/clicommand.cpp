#include "clicommand.h"
#include "sqlitestudio.h"
#include "qio.h"
#include "cli_config.h"
#include "cliutils.h"
#include "utils.h"
#include "clicommandfactory.h"

CliCommand::CliCommand()
{
}

CliCommand::~CliCommand()
{
}

void CliCommand::setup(CLI *cli)
{
    this->cli = cli;
    this->config = SQLiteStudio::getInstance()->getConfig();
    defineSyntax();
}

bool CliCommand::isAsyncExecution() const
{
    return false;
}

bool CliCommand::parseArgs(const QStringList& args)
{
    bool res = syntax.parse(args);

    if (!res)
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
    foreach (int id, syntax.getRegularArgumentCandidates())
        results += getCompletionValuesFor(id);

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

void CliCommand::printBox(const QString& str)
{
    int cols = getCliColumns();

    QStringList lines = str.split("\n");
    println(".---------------------------");
    foreach (const QString& line, lines)
    {
        foreach (const QString& lineWithMargin, applyMargin(line, cols - 3)) // 2 for "| " and 1 for final new line character
            println("| " + lineWithMargin);
    }

    println("`---------------------------");
}

void CliCommand::printUsage()
{
    println(tr("Usage: %1%2").arg(CFG_CLI.Console.CommandPrefixChar.get()).arg(usage()));
    println("");
}

QStringList CliCommand::getCompletionValuesFor(int id)
{
    QStringList results;
    if (id < 1000) // this base implementation is only for local enum values (>= 1000).
        return results;

    switch (static_cast<ArgIds>(id))
    {
        case CliCommand::DB_NAME:
            break;
        case CliCommand::DB_NAME_OR_FILE:
            break;
        case CliCommand::FILE_PATH:
            break;
        case CliCommand::DIR_PATH:
            break;
        case CliCommand::CMD_NAME:
            results += CliCommandFactory::getCommandNames();
            break;
        case CliCommand::PATTERN:
            results += "*";
        case CliCommand::STRING:
            break;
    }

    return results;
}

QString CliCommand::cmdName(const QString& cmd)
{
    return CFG_CLI.Console.CommandPrefixChar.get()+cmd;
}
