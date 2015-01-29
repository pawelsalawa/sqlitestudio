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
#include "uiconfig.h"
#include "sqlitestudio.h"
#include "uidebug.h"
#include "completionhelper.h"
#include "services/updatemanager.h"
#include "guiSQLiteStudio_global.h"
#include "coreSQLiteStudio_global.h"
#include "log.h"
#include "qio.h"
#include "translations.h"
#include "dialogs/languagedialog.h"
#include "services/pluginmanager.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QPluginLoader>
#include <QDebug>
#include <QMessageBox>

bool listPlugins = false;

QString uiHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages in console (accessible with F12)."));
    QCommandLineOption debugStdOutOption("debug-stdout", QObject::tr("Redirects debug messages into standard output (forces debug mode)."));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption sqlDebugOption("debug-sql", QObject::tr("Enables debugging of every single SQL query being sent to any database."));
    QCommandLineOption sqlDebugDbNameOption("debug-sql-db", QObject::tr("Limits SQL query messages to only the given <database>."), QObject::tr("database"));
    QCommandLineOption listPluginsOption("list-plugins", QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    parser.addOption(debugOption);
    parser.addOption(debugStdOutOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(sqlDebugOption);
    parser.addOption(sqlDebugDbNameOption);
    parser.addOption(listPluginsOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    setUiDebug(parser.isSet(debugOption) || parser.isSet(debugStdOutOption) || parser.isSet(sqlDebugOption), !parser.isSet(debugStdOutOption));
    CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);
    setSqlLoggingEnabled(parser.isSet(sqlDebugOption));
    if (parser.isSet(sqlDebugDbNameOption))
        setSqlLoggingFilter(parser.value(sqlDebugDbNameOption));

    if (parser.isSet(listPluginsOption))
        listPlugins = true;

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString::null;
}

bool updateRetryFunction(const QString& msg)
{
    QMessageBox mb(QMessageBox::Critical, QObject::tr("Error"), msg);
    mb.addButton(QMessageBox::Retry);
    mb.addButton(QMessageBox::Abort);
    return (mb.exec() == QMessageBox::Retry);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int retCode = 1;
    UpdateManager::setRetryFunction(updateRetryFunction);
    if (UpdateManager::handleUpdateOptions(a.arguments(), retCode))
    {
        if (retCode)
            QMessageBox::critical(nullptr, QObject::tr("Error"), UpdateManager::getStaticErrorMessage());

        return retCode;
    }

    qInstallMessageHandler(uiMessageHandler);

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

    MainWindow::getInstance();

    SQLITESTUDIO->initPlugins();

    if (listPlugins)
    {
        for (const PluginManager::PluginDetails& details : PLUGINS->getAllPluginDetails())
            qOut << details.name << " " << details.versionString << "\n";

        return 0;
    }

    IconManager::getInstance()->rescanResources();

    if (!LanguageDialog::didAskForDefaultLanguage())
    {
        LanguageDialog::askedForDefaultLanguage();
        QMap<QString, QString> langs = getAvailableLanguages();

        LanguageDialog dialog;
        dialog.setLanguages(langs);
        dialog.setSelectedLang(getConfigLanguageDefault());
        if (dialog.exec() == QDialog::Accepted)
            setDefaultLanguage(dialog.getSelectedLang());

        QProcess::startDetached(a.applicationFilePath(), QStringList());
        return 0;
    }

    MainWindow::getInstance()->restoreSession();
    MainWindow::getInstance()->show();

    if (!dbToOpen.isNull())
        MainWindow::getInstance()->openDb(dbToOpen);

    UPDATES->checkForUpdates();

    return a.exec();
}
