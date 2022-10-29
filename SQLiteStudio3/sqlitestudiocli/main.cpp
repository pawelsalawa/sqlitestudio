#include "cli.h"
#include "clicommandexecutor.h"
#include "commands/clicommand.h"
#include "cli_config.h"
#include "cliutils.h"
#include "qio.h"
#include "climsghandler.h"
#include "completionhelper.h"
#include "services/pluginmanager.h"
#include "sqlfileexecutor.h"
#include <QCoreApplication>
#include <QtGlobal>
#include <QCommandLineParser>
#include <QCommandLineOption>

bool listPlugins = false;
QString sqlScriptToExecute;

QString cliHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Command line interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages on standard error output."));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption listPluginsOption("list-plugins", QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    QCommandLineOption execSqlOption({"e", "execute-sql-file"},
                                     QObject::tr("Executes provided SQL file (including all rich features of SQLiteStudio's query executor) "
                                                 "on the specified database and quits. "
                                                 "The database parameter becomes mandatory if this option is used."),
                                     QObject::tr("SQL file"));

    parser.addOption(debugOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(listPluginsOption);
    parser.addOption(execSqlOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    if (parser.isSet(debugOption))
        setCliDebug(true);

    if (parser.isSet(execSqlOption))
        sqlScriptToExecute = parser.value(execSqlOption);

    if (parser.isSet(listPluginsOption))
        listPlugins = true;

    CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString();
}

int cliExecSqlFromFile(const QString& dbToOpen)
{
    if (dbToOpen.isEmpty())
    {
        qErr << QObject::tr("Database file argument is mandatory when executing SQL file.") << "\n";
        qErr.flush();
        return 1;
    }
    if (!CLI::getInstance()->openDbFile(dbToOpen))
    {
        qErr << QObject::tr("Could not open specified database for executing SQL file. You my try using -d option to find out more details.") << "\n";
        qErr.flush();
        return 1;
    }

    Db* db = CLI::getInstance()->getCurrentDb();

    SqlFileExecutor executor;
    executor.execSqlFromFile(db, sqlScriptToExecute, false, defaultCodecName(), false);
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SQLiteStudio");
    QCoreApplication::setOrganizationName("SalSoft");
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
        for (PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
            qOut << details.name << " " << details.versionString << "\n";

        return 0;
    }

    if (!sqlScriptToExecute.isNull())
        return cliExecSqlFromFile(dbToOpen);

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
