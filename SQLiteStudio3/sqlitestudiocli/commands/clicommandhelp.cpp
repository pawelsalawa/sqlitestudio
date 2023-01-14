#include "clicommandhelp.h"
#include "clicommandfactory.h"
#include "common/utils.h"
#include "cli_config.h"

void CliCommandHelp::execute()
{
    if (syntax.isArgumentSet(CMD_NAME))
        printHelp(syntax.getArgument(CMD_NAME));
    else
        printHelp();
}

QString CliCommandHelp::shortHelp() const
{
    return tr("shows this help message");
}

QString CliCommandHelp::fullHelp() const
{
    return tr(
                "Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.\n"
                "To see list of supported commands, type %2 without any arguments.\n\n"
                "When passing <command> name, you can skip special prefix character ('%3').\n\n"
                "You can always execute any command with exactly single '--help' option to see help for that command. "
                "It's an alternative for typing: %1 <command>."
                ).arg(cmdName("help"), cmdName("help"), CFG_CLI.Console.CommandPrefixChar.get(), cmdName("help"));
}

void CliCommandHelp::defineSyntax()
{
    syntax.setName("help");
    syntax.addArgument(CMD_NAME, tr("command", "CLI command syntax"), false);
}

void CliCommandHelp::printHelp(const QString& cmd)
{
    QString cmdStr = cmd.startsWith(".") ? cmd.mid(1) : cmd;
    CliCommand* command = CliCommandFactory::getCommand(cmdStr);
    if (!command)
    {
        println(tr("No such command: %1").arg(cmd));
        println(tr("Type '%1' for list of available commands.").arg(cmdName("help")));
        println("");
        return;
    }
    command->defineSyntax();
    QStringList aliases = command->aliases();
    QString prefix = CFG_CLI.Console.CommandPrefixChar.get();

    QString msg;
    msg += tr("Usage: %1%2").arg(prefix, command->usage(cmdStr));
    msg += "\n";
    if (aliases.size() > 0)
    {
        if (aliases.contains(cmdStr))
        {
            aliases.removeOne(cmdStr);
            aliases << command->getName();
        }

        msg += tr("Aliases: %1").arg(prefix + aliases.join(", " + prefix));
        msg += "\n";
    }
    msg += "\n";
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
    QStringList msgList;
    for (const QString& cmd : names)
    {
        msgList << (CFG_CLI.Console.CommandPrefixChar.get() + pad(cmd, width, ' ') + " - " + allCommands[cmd]->shortHelp());
        delete allCommands[cmd];
    }
    printBox(msgList.join("\n"));
    printHelp("help");
}
