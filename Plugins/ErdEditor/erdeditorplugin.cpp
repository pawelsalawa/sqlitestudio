#include "erdeditorplugin.h"
#include "common/global.h"
#include "mainwindow.h"
#include "erdwindow.h"
#include "dbtree/dbtree.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "changes/erdchange.h"
#include <QAction>

ErdEditorPlugin* ErdEditorPlugin::instance = nullptr;

bool ErdEditorPlugin::init()
{
    SQLS_INIT_RESOURCE(erdeditor);
    
    ErdWindow::staticInit();
    instance = this;

    openErdEditorAction = new QAction(QIcon(":/icons/erdeditor.svg"), tr("Open ERD editor"), this);
    connect(openErdEditorAction, SIGNAL(triggered()), this, SLOT(openEditor()));

    QAction* ddlHistoryAction = MAINWINDOW->getAction(MainWindow::OPEN_DDL_HISTORY);
    MAINWINDOW->getToolBar(MainWindow::TOOLBAR_MAIN)->insertAction(ddlHistoryAction, openErdEditorAction);
    MAINWINDOW->getToolsMenu()->insertAction(ddlHistoryAction, openErdEditorAction);

    qRegisterMetaType<ErdChange*>();

    return GenericPlugin::init();
}

void ErdEditorPlugin::deinit()
{
    QList<ErdWindow*> windows = MDIAREA->getMdiChilds<ErdWindow>();
    for (ErdWindow* win : windows)
        win->getMdiWindow()->close();

    MAINWINDOW->getToolBar(MainWindow::TOOLBAR_MAIN)->removeAction(openErdEditorAction);
    ErdWindow::staticCleanup();
    SQLS_CLEANUP_RESOURCE(erdeditor);
}

QString ErdEditorPlugin::getConfigUiForm() const
{
    return "ErdConfig";
}

CfgMain* ErdEditorPlugin::getMainUiConfig()
{
    return &cfg;
}

void ErdEditorPlugin::configDialogOpen()
{

}

void ErdEditorPlugin::configDialogClosed()
{

}

void ErdEditorPlugin::openEditor()
{
    Db* db = DBTREE->getSelectedOpenDb();
    if (!db)
        return;

    SchemaResolver resolver(db);
    QStringList tables = resolver.getTables();
    if (tables.length() > CFG_ERD.Erd.MaxTableLimit.get())
    {
        NOTIFY_MANAGER->error(
            tr("ERD editor cannot open because the database contains %1 tables, exceeding the configured limit of %2 tables. You can increase this limit in the settings, but higher values may slow down or freeze the application.")
                .arg(QString::number(tables.length()), QString::number(CFG_ERD.Erd.MaxTableLimit.get())));
        return;
    }

    MAINWINDOW->openMdiWindow<ErdWindow>(db);
}
