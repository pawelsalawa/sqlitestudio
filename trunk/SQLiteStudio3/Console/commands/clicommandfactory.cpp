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

QHash<QString,CliCommandFactory::CliCommandCreatorFunc> CliCommandFactory::mapping;

#define REGISTER_CMD(Str, Cmd) mapping[Str] = []() -> CliCommand* {return new Cmd();}

void CliCommandFactory::init()
{
    REGISTER_CMD("add", CliCommandAdd);
    REGISTER_CMD("remove", CliCommandRemove);
    REGISTER_CMD("exit", CliCommandExit);
    REGISTER_CMD("quit", CliCommandExit);
    REGISTER_CMD("dblist", CliCommandDbList);
    REGISTER_CMD("use", CliCommandUse);
    REGISTER_CMD("open", CliCommandOpen);
    REGISTER_CMD("close", CliCommandClose);
    REGISTER_CMD("query", CliCommandSql);
}

CliCommand *CliCommandFactory::getCommand(const QString &cmdName)
{
    if (!mapping.contains(cmdName))
        return nullptr;

    return mapping[cmdName]();
}
