#include "clicommandexecutor.h"
#include "commands/clicommand.h"

CliCommandExecutor::CliCommandExecutor(QObject *parent) :
    QObject(parent)
{
}

void CliCommandExecutor::execCommand(CliCommand* cmd, QStringList args)
{
    cmd->execute(args);
    delete cmd;
    emit executionComplete();
}
