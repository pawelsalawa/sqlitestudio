#include "clicommand.h"
#include "sqlitestudio.h"
#include "qio.h"
#include "cli_config.h"
#include "cliutils.h"
#include "utils.h"

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

QString CliCommand::cmdName(const QString& cmd)
{
    return CFG_CLI.Console.CommandPrefixChar.get()+cmd;
}
