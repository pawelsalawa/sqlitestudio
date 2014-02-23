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

QHash<QString,CliCommandCreatorFunc> CliCommandFactory::mapping;

void CliCommandFactory::init()
{
    registerCommand("add", (CliCommandCreatorFunc)&CliCommandAdd::create);
    registerCommand("remove", (CliCommandCreatorFunc)&CliCommandRemove::create);
    registerCommand("exit", (CliCommandCreatorFunc)&CliCommandExit::create);
    registerCommand("quit", (CliCommandCreatorFunc)&CliCommandExit::create);
    registerCommand("dblist", (CliCommandCreatorFunc)&CliCommandDbList::create);
    registerCommand("use", (CliCommandCreatorFunc)&CliCommandUse::create);
    registerCommand("open", (CliCommandCreatorFunc)&CliCommandOpen::create);
    registerCommand("close", (CliCommandCreatorFunc)&CliCommandClose::create);
}

CliCommand *CliCommandFactory::getCommand(const QString &cmdName)
{
    if (!mapping.contains(cmdName))
        return nullptr;

    return mapping[cmdName]();
}

CliCommand *CliCommandFactory::getSqlCommand()
{
    return CliCommandSql::create();
}

void CliCommandFactory::registerCommand(const QString &name, CliCommandCreatorFunc creator)
{
    mapping[name] = creator;
}


