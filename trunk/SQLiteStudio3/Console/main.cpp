#include "cli.h"
#include "clicommandexecutor.h"
#include "sqlitestudio.h"
#include "commands/clicommand.h"
#include "cli_config.h"
#include "qio.h"
#include "climsghandler.h"
#include <QCoreApplication>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qOut << QString("SQLiteStudio (%1)\n------------------------\n\n").arg("x.x.x");
    qOut.flush();

    qInstallMessageHandler(cliMessageHandler);

    CliResultsDisplay::staticInit();

    SQLiteStudio::getInstance()->init(a.arguments());

    CliCommandExecutor executor;

    CLI cli;
    QObject::connect(&cli, &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, &cli, &CLI::executionComplete);

    cli.start();

    return a.exec();
}
