#include "mainwindow.h"
#include "iconmanager.h"
#include "dbtree/dbtreeitem.h"
#include "datagrid/sqlquerymodelcolumn.h"
#include "datagrid/sqlquerymodel.h"
#include "sqleditor.h"
#include "windows/editorwindow.h"
#include "windows/tablewindow.h"
#include "windows/viewwindow.h"
#include "dataview.h"
#include "dbtree/dbtree.h"
#include "multieditor/multieditordatetime.h"
#include "multieditor/multieditortime.h"
#include "multieditor/multieditordate.h"
#include "multieditor/multieditorbool.h"
#include "uidebug.h"
#include "completionhelper.h"
#include "services/updatemanager.h"
#include "guiSQLiteStudio_global.h"
#include "log.h"
#include "qio.h"
#include "translations.h"
#include "dialogs/languagedialog.h"
#include "dialogs/triggerdialog.h"
#include "services/pluginmanager.h"
#include "singleapplication/singleapplication.h"
#include "services/impl/configimpl.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QPluginLoader>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QSettings>
#ifdef Q_OS_WIN
#   include <windef.h>
#   include <windows.h>
#endif

static bool listPlugins = false;

QString uiHandleCmdLineArgs(bool applyOptions = true)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages in console (accessible with F12)."));
    QCommandLineOption debugStdOutOption("debug-stdout", QObject::tr("Redirects debug messages into standard output (forces debug mode)."));
    QCommandLineOption debugFileOption("debug-file", QObject::tr("Redirects debug messages into given file (forces debug mode)."), QObject::tr("log file"));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption sqlDebugOption("debug-sql", QObject::tr("Enables debugging of every single SQL query being sent to any database."));
    QCommandLineOption sqlDebugDbNameOption("debug-sql-db", QObject::tr("Limits SQL query messages to only the given <database>."), QObject::tr("database"));
    QCommandLineOption executorDebugOption("debug-query-executor", QObject::tr("Enables debugging of SQLiteStudio's query executor."));
    QCommandLineOption listPluginsOption("list-plugins", QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    QCommandLineOption masterConfigOption("master-config", QObject::tr("Points to the master configuration file. Read manual at wiki page for more details."), QObject::tr("SQLiteStudio settings file"));
    parser.addOption(debugOption);
    parser.addOption(debugStdOutOption);
    parser.addOption(debugFileOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(sqlDebugOption);
    parser.addOption(sqlDebugDbNameOption);
    parser.addOption(executorDebugOption);
    parser.addOption(masterConfigOption);
    parser.addOption(listPluginsOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    if (applyOptions)
    {
        bool enableDebug = parser.isSet(debugOption) || parser.isSet(debugStdOutOption) || parser.isSet(sqlDebugOption) || parser.isSet(debugFileOption);
        setUiDebug(enableDebug, !parser.isSet(debugStdOutOption), parser.value(debugFileOption));
        CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);
        setSqlLoggingEnabled(parser.isSet(sqlDebugOption));
        setExecutorLoggingEnabled(parser.isSet(executorDebugOption));
        if (parser.isSet(sqlDebugDbNameOption))
            setSqlLoggingFilter(parser.value(sqlDebugDbNameOption));

        if (parser.isSet(listPluginsOption))
            listPlugins = true;

        if (parser.isSet(masterConfigOption))
            Config::setMasterConfigFile(parser.value(masterConfigOption));
    }

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString();
}

void initHighDpi()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qgetenv("QT_ENABLE_HIGHDPI_SCALING").isEmpty() && qgetenv("QT_AUTO_SCREEN_SCALE_FACTOR").isEmpty())
    {
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }
#else
    // Qt 6 handles HighDPI by default
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && defined(Q_OS_WIN)
    if (qgetenv("QT_SCALE_FACTOR_ROUNDING_POLICY").isEmpty())
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

bool shouldAllowMultipleSessions()
{
    QVariant allowMultipleSessionsValue = Config::getSettings()->value(MainWindow::ALLOW_MULTIPLE_SESSIONS_SETTING);
    return allowMultipleSessionsValue.isValid() && allowMultipleSessionsValue.toBool();
}

int main(int argc, char *argv[])
{
    initHighDpi();
    QCoreApplication::setApplicationName("SQLiteStudio");
    QCoreApplication::setOrganizationName("SalSoft");
    QCoreApplication::setApplicationVersion(SQLITESTUDIO->getVersionString());

    SingleApplication a(argc, argv, true, SingleApplication::ExcludeAppPath|SingleApplication::ExcludeAppVersion|SingleApplication::User);

    if (!shouldAllowMultipleSessions() && a.isSecondary()) {
#ifdef Q_OS_WIN
        AllowSetForegroundWindow(DWORD( a.primaryPid()));
#endif
        QString dbToOpen = uiHandleCmdLineArgs();
        a.sendMessage(serializeToBytes(dbToOpen));
        return 0;
    }

    qInstallMessageHandler(uiMessageHandler);

    Config::setAskUserForConfigDirFunc([]() -> QString
    {
       return QFileDialog::getExistingDirectory(nullptr, QObject::tr("Select configuration directory"), QString(), QFileDialog::ShowDirsOnly);
    });

    QString dbToOpen = uiHandleCmdLineArgs();

    DbTreeItem::initMeta();
    SqlQueryModelColumn::initMeta();
    SqlQueryModel::staticInit();

    SQLITESTUDIO->setInitialTranslationFiles({"coreSQLiteStudio", "guiSQLiteStudio", "sqlitestudio"});
    SQLITESTUDIO->init(a.arguments(), true);
    IconManager::getInstance()->init();
    DbTree::staticInit();
    DataView::staticInit();
    EditorWindow::staticInit();
    TableWindow::staticInit();
    ViewWindow::staticInit();
    MultiEditorDateTime::staticInit();
    MultiEditorTime::staticInit();
    MultiEditorDate::staticInit();
    MultiEditorBool::staticInit();
    TriggerDialog::staticInit();
    SqlEditor::staticInit();

    MainWindow* mainWin = MAINWINDOW;

    QObject::connect(&a, &SingleApplication::receivedMessage, mainWin, &MainWindow::messageFromSecondaryInstance);

    SQLITESTUDIO->initPlugins();

    if (listPlugins)
    {
        for (const PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
            qOut << details.name << " " << details.versionString << "\n";

        return 0;
    }

    if (!LanguageDialog::didAskForDefaultLanguage() && !SQLITESTUDIO->getConfig()->isInMemory())
    {
        LanguageDialog::askedForDefaultLanguage();
        QMap<QString, QString> langs = getAvailableLanguages();

        LanguageDialog dialog;
        dialog.setLanguages(langs);
        dialog.setSelectedLang(getConfigLanguageDefault());
        if (dialog.exec() == QDialog::Accepted)
            setDefaultLanguage(dialog.getSelectedLang());

        QProcess::startDetached(qApp->arguments().at(0), qApp->arguments().mid(1));
        return 0;
    }

    // Shortcuts titles needs to be retranslated, because their titles were set initially in global scope,
    // while translation files were not loaded yet. Now they are.
    ExtActionContainer::refreshShortcutTranslations();

    MainWindow::getInstance()->restoreSession();
    MainWindow::getInstance()->show();

    if (!dbToOpen.isNull())
        MainWindow::getInstance()->openDb(dbToOpen);

#ifdef PORTABLE_CONFIG
    UPDATES->checkForUpdates();
#endif

    return a.exec();
}
