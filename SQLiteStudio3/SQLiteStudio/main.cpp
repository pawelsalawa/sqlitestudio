#include "mainwindow.h"
#include "sqlitestudio.h"
#include "iconmanager.h"
#include "dbtree/dbtreeitem.h"
#include "datagrid/sqlquerymodelcolumn.h"
#include "datagrid/sqlquerymodel.h"
#include "sqleditor.h"
#include "windows/editorwindow.h"
#include "windows/tablewindow.h"
#include "windows/viewwindow.h"
#include "dataview.h"
#include "multieditor/multieditordatetime.h"
#include "multieditor/multieditortime.h"
#include "multieditor/multieditordate.h"
#include "multieditor/multieditorbool.h"
#include "uiconfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DbTreeItem::initMeta();
    SqlQueryModelColumn::initMeta();
    SqlQueryModel::staticInit();
    SQLiteStudio::getInstance()->init(a.arguments());
    IconManager::getInstance();
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

    return a.exec();
}
