#include "printing.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "windows/editorwindow.h"
#include "dataview.h"
#include <QPrinter>
#include <QPrintDialog>

bool Printing::init()
{
    Q_INIT_RESOURCE(printing);
    printResultsAction = new QAction(QIcon(":/icons/printer.png"), tr("Print results"), this);
    ExtActionContainer::insertActionBefore<DataView>(printResultsAction, DataView::REFRESH_DATA, DataView::TOOLBAR_GRID);
    return true;
}

void Printing::deinit()
{
    Q_CLEANUP_RESOURCE(printing);
    ExtActionContainer::removeAction<DataView>(printResultsAction, DataView::TOOLBAR_GRID);
    safe_delete(printResultsAction);
}
