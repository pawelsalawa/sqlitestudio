#include "erdeditorplugin.h"
#include "common/global.h"
#include "mainwindow.h"
#include "erdwindow.h"
#include "dbtree/dbtree.h"
#include <QAction>

CFG_DEFINE(ErdConfig)

bool ErdEditorPlugin::init()
{
    SQLS_INIT_RESOURCE(erdeditor);
    
    ErdWindow::staticInit();

    openErdEditorAction = new QAction(QIcon(":/icons/erdeditor.png"), tr("Open ERD editor"), this);
    connect(openErdEditorAction, SIGNAL(triggered()), this, SLOT(openEditor()));

    QAction* ddlHistoryAction = MAINWINDOW->getAction(MainWindow::OPEN_DDL_HISTORY);
    MAINWINDOW->getToolBar(MainWindow::TOOLBAR_MAIN)->insertAction(ddlHistoryAction, openErdEditorAction);
    MAINWINDOW->getToolsMenu()->insertAction(ddlHistoryAction, openErdEditorAction);

    return true;
}

void ErdEditorPlugin::deinit()
{
    MAINWINDOW->getToolBar(MainWindow::TOOLBAR_MAIN)->removeAction(openErdEditorAction);
    CFG_DELETE_INSTANCE(ErdConfig);
    SQLS_CLEANUP_RESOURCE(erdeditor);
}

void ErdEditorPlugin::openEditor()
{
    Db* db = DBTREE->getSelectedOpenDb();
    MAINWINDOW->openMdiWindow<ErdWindow>(db);
}
