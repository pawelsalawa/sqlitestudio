#include "clicommandfactory.h"
#include "cli.h"
#include "clicommandadd.h"
#include "clicommandremove.h"
#include "clicommandexit.h"
#include "clicommanddblist.h"
#include "clicommanduse.h"
#include "clicommandopen.h"
#include "clicommandclose.h"
#include "clicommandsql.h"
#include "clicommandhelp.h"
#include "clicommandtables.h"
#include "clicommandmode.h"
#include "clicommandnullvalue.h"
#include "clicommandhistory.h"
#include "clicommanddir.h"
#include "clicommandpwd.h"
#include "clicommandcd.h"
#include "clicommandtree.h"
#include <QDebug>

QHash<QString,CliCommandFactory::CliCommandCreatorFunc> CliCommandFactory::mapping;

#define REGISTER_CMD(Cmd) registerCommand([]() -> CliCommand* {return new Cmd();})

void CliCommandFactory::init()
{
    REGISTER_CMD(CliCommandAdd);
    REGISTER_CMD(CliCommandRemove);
    REGISTER_CMD(CliCommandExit);
    REGISTER_CMD(CliCommandDbList);
    REGISTER_CMD(CliCommandUse);
    REGISTER_CMD(CliCommandOpen);
    REGISTER_CMD(CliCommandClose);
    REGISTER_CMD(CliCommandSql);
    REGISTER_CMD(CliCommandHelp);
    REGISTER_CMD(CliCommandTables);
    REGISTER_CMD(CliCommandMode);
    REGISTER_CMD(CliCommandNullValue);
    REGISTER_CMD(CliCommandHistory);
    REGISTER_CMD(CliCommandDir);
    REGISTER_CMD(CliCommandPwd);
    REGISTER_CMD(CliCommandCd);
    REGISTER_CMD(CliCommandTree);
}

CliCommand *CliCommandFactory::getCommand(const QString &cmdName)
{
    if (!mapping.contains(cmdName))
        return nullptr;

    return mapping[cmdName]();
}

QHash<QString,CliCommand*> CliCommandFactory::getAllCommands()
{
    QHash<QString,CliCommand*> results;
    QHashIterator<QString,CliCommandFactory::CliCommandCreatorFunc> it(mapping);
    while (it.hasNext())
    {
        it.next();
        results[it.key()] = it.value()();
    }

    return results;
}

void CliCommandFactory::registerCommand(CliCommandCreatorFunc func)
{
    CliCommand* cmd = func();
    cmd->defineSyntax();

    mapping[cmd->getName()] = func;
    foreach (const QString& alias, cmd->aliases())
        mapping[alias] = func;

    delete cmd;
}
