#include "clicommandexecutor.h"
#include "commands/clicommand.h"

CliCommandExecutor::CliCommandExecutor(QObject *parent) :
    QObject(parent)
{
}

void CliCommandExecutor::execCommand(CliCommand* cmd)
{
    if (cmd->isAsyncExecution())
        connect(cmd, SIGNAL(execComplete()), this, SLOT(asyncExecutionComplete()));

    cmd->execute();
    if (!cmd->isAsyncExecution())
    {
        delete cmd;
        emit executionComplete();
    }
}

void CliCommandExecutor::asyncExecutionComplete()
{
    sender()->deleteLater();
    emit executionComplete();
}
