#include "clicommandexecutor.h"
#include "commands/clicommand.h"

CliCommandExecutor::CliCommandExecutor(QObject *parent) :
    QObject(parent)
{
}

void CliCommandExecutor::execCommand(CliCommand* cmd, QStringList args)
{
    connect(cmd, SIGNAL(execComplete()), this, SLOT(asyncExecutionComplete()));
    bool async = cmd->execute(args);
    if (!async)
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
