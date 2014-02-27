#include "clicommandhelp.h"
#include "clicommandfactory.h"
#include "utils.h"
#include "cli_config.h"

bool CliCommandHelp::execute(QStringList args)
{
    if (args.size() == 1)
        printHelp(args[0]);
    else
        printHelp();

    return false;
}

bool CliCommandHelp::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    return true;
}

QString CliCommandHelp::shortHelp() const
{
    return tr("shows this help message");
}

QString CliCommandHelp::fullHelp() const
{
    return tr(
                "Use .help to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.\n"
                "To see list of supported commands, type .help without any arguments."
             );
}

QString CliCommandHelp::usage() const
{
    return tr("help [<command>]");
}

void CliCommandHelp::printHelp(const QString& cmd)
{
    QString cmdStr = cmd.startsWith(".") ? cmd.mid(1) : cmd;
    CliCommand* command = CliCommandFactory::getCommand(cmdStr);
    if (!command)
    {
        println(tr("No such command: %1").arg(cmd));
        println(tr("Type '.help' for list of available commands."));
        println("");
        return;
    }
    QString msg;
    msg += tr("Usage: %1%2").arg(CFG_CLI.General.CommandPrefixChar.get()).arg(command->usage());
    msg += "\n\n";
    msg += command->fullHelp();
    delete command;

    printBox(msg);
}

void CliCommandHelp::printHelp()
{
    QHash<QString, CliCommand*> allCommands = CliCommandFactory::getAllCommands();
    QStringList names = allCommands.keys();
    int width = longest(names).size();

    names.sort();
    QString msg;
    foreach (const QString& cmd, names)
    {
        msg += CFG_CLI.General.CommandPrefixChar.get() + pad(cmd, width, ' ') + " - " + allCommands[cmd]->shortHelp() + "\n";
        delete allCommands[cmd];
    }
    printBox(msg);
}
