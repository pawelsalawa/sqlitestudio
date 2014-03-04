#include "cli.h"
#include "clicommandexecutor.h"
#include "sqlitestudio.h"
#include "commands/clicommand.h"
#include "cli_config.h"
#include "qio.h"
#include "climsghandler.h"
#include <QCoreApplication>
#include <QtGlobal>
#include <QCommandLineParser>
#include <QCommandLineOption>

QString cliHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Command line interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("enables debug messages on standard error output."));
    parser.addOption(debugOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("database file to open"));

    parser.process(qApp->arguments());

    if (parser.isSet(debugOption))
        setCliDebug(true);

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString::null;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SQLiteStudio");
    QCoreApplication::setApplicationVersion("x.x.x");

    qInstallMessageHandler(cliMessageHandler);

    QString dbToOpen = cliHandleCmdLineArgs();

    CliResultsDisplay::staticInit();

    SQLiteStudio::getInstance()->init(a.arguments());

    CliCommandExecutor executor;

    CLI cli;
    QObject::connect(&cli, &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, &cli, &CLI::executionComplete);

    if (!dbToOpen.isEmpty())
        cli.openDbFile(dbToOpen);

    cli.start();

    return a.exec();
}
