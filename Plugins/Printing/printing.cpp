#include "printing.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "windows/editorwindow.h"
#include "dataview.h"
#include "common/extactionprototype.h"
#include <QPrinter>
#include <QPrintDialog>

bool Printing::init()
{
    Q_INIT_RESOURCE(printing);
    printDataAction = new ExtActionPrototype(QIcon(":/icons/printer.png"), tr("Print data"), this);
    separatorAction = new ExtActionPrototype(this);
    printQueryAction = new ExtActionPrototype(QIcon(":/icons/printer.png"), tr("Print query"), this);

    DataView::insertActionAfter(printDataAction, DataView::LAST_PAGE);
    DataView::insertActionAfter(separatorAction, DataView::LAST_PAGE);
    EditorWindow::insertActionAfter(printQueryAction, EditorWindow::EXPORT_RESULTS);
    return true;
}

void Printing::deinit()
{
    Q_CLEANUP_RESOURCE(printing);
    DataView::removeAction(printDataAction);
    DataView::removeAction(separatorAction);
    EditorWindow::removeAction(printQueryAction);
    safe_delete(printDataAction);
    safe_delete(separatorAction);
    safe_delete(printQueryAction);
}
