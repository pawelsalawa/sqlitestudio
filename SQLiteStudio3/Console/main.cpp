#include <QCoreApplication>

#include "cli.h"
#include "clicommandexecutor.h"
#include "sqlitestudio.h"
#include "commands/clicommand.h"
#include "cli_config.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SQLiteStudio::getInstance()->init(a.arguments());

    CliCommandExecutor executor;
    CLI cli;
    QObject::connect(&cli, &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, &cli, &CLI::executionComplete);

    cli.start();

    return a.exec();
}
