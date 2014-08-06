#include "cli.h"
#include "clicommandexecutor.h"
#include "sqlitestudio.h"
#include "commands/clicommand.h"
#include "cli_config.h"
#include "cliutils.h"
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
    QCoreApplication::setApplicationVersion(SQLITESTUDIO->getVersionString());

    qInstallMessageHandler(cliMessageHandler);

    QString dbToOpen = cliHandleCmdLineArgs();

    CliResultsDisplay::staticInit();
    initCliUtils();

    SQLITESTUDIO->init(a.arguments(), false);
    SQLITESTUDIO->initPlugins();

    CliCommandExecutor executor;

    QObject::connect(CLI::getInstance(), &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, CLI::getInstance(), &CLI::executionComplete);

    if (!dbToOpen.isEmpty())
        CLI::getInstance()->openDbFile(dbToOpen);

    CLI::getInstance()->start();

    return a.exec();
}
