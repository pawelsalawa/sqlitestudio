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

namespace CliOpts
{
    bool listPlugins = false;
    QString sqlScriptToExecute;
    QString dbToOpen;
    QString sqlScriptCodec;
    bool ignoreErrors = false;
}

bool cliHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Command line interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages on standard error output."));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption listPluginsOption({"lp", "list-plugins"}, QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    QCommandLineOption execSqlOption({"e", "execute-sql-file"},
                                     QObject::tr("Executes provided SQL file (including all rich features of SQLiteStudio's query executor) "
                                                 "on the specified database file and quits. "
                                                 "The database parameter becomes mandatory if this option is used."),
                                     QObject::tr("SQL file"));
    QCommandLineOption sqlFileCodecOption({"c", "file-codec"}, QObject::tr("Character encoding to use when reading SQL file (-e option). "
                                                                               "Use -cl to list available codecs. "
                                                                               "Defaults to %1.").arg(defaultCodecName()),
                                          QObject::tr("codec"));
    QCommandLineOption codecListOption({"lc", "list-codecs"}, QObject::tr("Lists available codecs to be used with -c option and quits."));
    QCommandLineOption ignoreErrorsOption({"ie", "ignore-errors"},
                                          QObject::tr("When used together with -e option, the execution will not stop on an error, "
                                                      "but rather continue until the end, ignoring errors."));

    parser.addOption(debugOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(listPluginsOption);
    parser.addOption(execSqlOption);
    parser.addOption(sqlFileCodecOption);
    parser.addOption(codecListOption);
    parser.addOption(ignoreErrorsOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    if (parser.isSet(debugOption))
        setCliDebug(true);

    if (parser.isSet(codecListOption))
    {
        for (QString& codec : textCodecNames())
            qOut << codec << "\n";

        qOut.flush();
        return true;
    }

    if (parser.isSet((sqlFileCodecOption)))
    {
        CliOpts::sqlScriptCodec = parser.value(sqlFileCodecOption);
        if (!textCodecNames().contains(CliOpts::sqlScriptCodec))
        {
            qErr << QObject::tr("Invalid codec: %1. Use -cl option to list available codecs.").arg(CliOpts::sqlScriptCodec) << "\n";
            qErr.flush();
            return true;
        }
    }
    else
        CliOpts::sqlScriptCodec = defaultCodecName();

    if (parser.isSet(ignoreErrorsOption))
        CliOpts::ignoreErrors = true;

    if (parser.isSet(execSqlOption))
        CliOpts::sqlScriptToExecute = parser.value(execSqlOption);

    if (parser.isSet(listPluginsOption))
        CliOpts::listPlugins = true;

    CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        CliOpts::dbToOpen = args[0];

    return false;
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
        qErr << QObject::tr("Could not open specified database for executing SQL file. You may try using -d option to find out more details.") << "\n";
        qErr.flush();
        return 1;
    }

    Db* db = CLI::getInstance()->getCurrentDb();

    SqlFileExecutor executor;
    executor.execSqlFromFile(db, CliOpts::sqlScriptToExecute, CliOpts::ignoreErrors, CliOpts::sqlScriptCodec, false);
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SQLiteStudio");
    QCoreApplication::setOrganizationName("SalSoft");
    QCoreApplication::setApplicationVersion(SQLITESTUDIO->getVersionString());

    qInstallMessageHandler(cliMessageHandler);

    if (cliHandleCmdLineArgs())
        return 0;

    initCliUtils();
    CliResultsDisplay::staticInit();

    SQLITESTUDIO->setInitialTranslationFiles({"coreSQLiteStudio", "sqlitestudiocli"});
    SQLITESTUDIO->init(a.arguments(), false);
    SQLITESTUDIO->initPlugins();

    if (CliOpts::listPlugins)
    {
        for (PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
            qOut << details.name << " " << details.versionString << "\n";

        return 0;
    }

    if (!CliOpts::sqlScriptToExecute.isNull())
        return cliExecSqlFromFile(CliOpts::dbToOpen);

    CliCommandExecutor executor;

    QObject::connect(CLI::getInstance(), &CLI::execCommand, &executor, &CliCommandExecutor::execCommand);
    QObject::connect(&executor, &CliCommandExecutor::executionComplete, CLI::getInstance(), &CLI::executionComplete);

    if (!CliOpts::dbToOpen.isEmpty())
        CLI::getInstance()->openDbFile(CliOpts::dbToOpen);

    CLI::getInstance()->start();
    int res = a.exec();
    CLI::dispose();
    return res;
}
