#include "cli.h"
#include "clicommandexecutor.h"
#include "sqlitestudio.h"
#include "commands/clicommand.h"
#include "cli_config.h"
#include "cliutils.h"
#include "qio.h"
#include "climsghandler.h"
#include "completionhelper.h"
#include "services/updatemanager.h"
#include "services/pluginmanager.h"
#include <QCoreApplication>
#include <QtGlobal>
#include <QCommandLineParser>
#include <QCommandLineOption>

bool listPlugins = false;

QString cliHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Command line interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages on standard error output."));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption listPluginsOption("list-plugins", QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    parser.addOption(debugOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(listPluginsOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    if (parser.isSet(debugOption))
        setCliDebug(true);

    if (parser.isSet(listPluginsOption))
        listPlugins = true;

    CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString::null;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef PORTABLE_CONFIG
    int retCode = 1;
    if (UpdateManager::handleUpdateOptions(a.arguments(), retCode))
        return retCode;
#endif

    QCoreApplication::setApplicationName("SQLiteStudio");
    QCoreApplication::setApplicationVersion(SQLITESTUDIO->getVersionString());

    qInstallMessageHandler(cliMessageHandler);

    QString dbToOpen = cliHandleCmdLineArgs();

    CliResultsDisplay::staticInit();
    initCliUtils();

    SQLITESTUDIO->setInitialTranslationFiles({"coreSQLiteStudio", "sqlitestudiocli"});
    SQLITESTUDIO->init(a.arguments(), false);
    SQLITESTUDIO->initPlugins();

    if (listPlugins)
    {
        for (const PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
            qOut << details.name << " " << details.versionString << "\n";

        return 0;
    }

    CliCommandExecutor executor;

    QObject::connect(CLI::getInstance(), &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, CLI::getInstance(), &CLI::executionComplete);

    if (!dbToOpen.isEmpty())
        CLI::getInstance()->openDbFile(dbToOpen);

    CLI::getInstance()->start();
    int res = a.exec();
    CLI::dispose();
    return res;
}
