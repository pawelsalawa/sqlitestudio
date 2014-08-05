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
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QSplashScreen>
#include <QThread>

#include <QPluginLoader>
#include <QDebug>

QString uiHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("enables debug messages in console (accessible with F12)."));
    QCommandLineOption debugStdOutOption("debug-stdout", QObject::tr("redirects debug messages into standard output (forces debug mode)."));
    parser.addOption(debugOption);
    parser.addOption(debugStdOutOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("database file to open"));

    parser.process(qApp->arguments());

    setUiDebug(parser.isSet(debugOption) || parser.isSet(debugStdOutOption), !parser.isSet(debugStdOutOption));


    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString::null;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qInstallMessageHandler(uiMessageHandler);

    QString dbToOpen = uiHandleCmdLineArgs();

    DbTreeItem::initMeta();
    SqlQueryModelColumn::initMeta();
    SqlQueryModel::staticInit();

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

    MainWindow::getInstance()->restoreSession();
    MainWindow::getInstance()->show();

    if (!dbToOpen.isNull())
        MainWindow::getInstance()->openDb(dbToOpen);

    return a.exec();
}
