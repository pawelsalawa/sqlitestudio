#include "printing.h"
#include "printingexport.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "windows/editorwindow.h"
#include "dataview.h"
#include "common/extactionprototype.h"
#include "datagrid/sqlquerymodel.h"
#include "exportworker.h"
#include "services/notifymanager.h"
#include "sqleditor.h"
#include "translations.h"
#include <QDebug>
#include <QPrinter>
#include <QPrintDialog>
#include <QThreadPool>

bool Printing::init()
{
    SQLS_INIT_RESOURCE(printing);

    printingExport = new PrintingExport();
    bool printingExportInit = printingExport->init();
    if (!printingExportInit)
        return false;

    loadTranslation("Printing");

    printingConfig = new ExportManager::StandardExportConfig();
    printingConfig->exportData = true;
    printingConfig->exportTableIndexes = false;
    printingConfig->exportTableTriggers = false;
    printingConfig->codec = defaultCodecName();

    QIcon printerIcon(":/icons/printer.svg");
    printDataAction = new ExtActionPrototype(printerIcon, tr("Print data"), this);
    separatorAction = new ExtActionPrototype(this);
    printQueryAction = new ExtActionPrototype(printerIcon, tr("Print query"), this);

    connect(printDataAction, SIGNAL(triggered(ExtActionContainer*,int)), this, SLOT(dataPrintRequested(ExtActionContainer*)));
    connect(printQueryAction, SIGNAL(triggered(ExtActionContainer*,int)), this, SLOT(queryPrintRequested(ExtActionContainer*)));

    DataView::insertActionAfter(printDataAction, DataView::LAST_PAGE);
    DataView::insertActionAfter(separatorAction, DataView::LAST_PAGE);
    EditorWindow::insertActionAfter(printQueryAction, EditorWindow::EXPORT_RESULTS);

    return true;
}

void Printing::deinit()
{
    printingExport->deinit();

    DataView::removeAction(printDataAction);
    DataView::removeAction(separatorAction);
    EditorWindow::removeAction(printQueryAction);
    safe_delete(printingExport);
    safe_delete(printDataAction);
    safe_delete(separatorAction);
    safe_delete(printQueryAction);
    SQLS_CLEANUP_RESOURCE(printing);
}

void Printing::dataPrintRequested(ExtActionContainer* actionContainer)
{
    DataView* dataView = dynamic_cast<DataView*>(actionContainer);
    if (!dataView)
    {
        qCritical() << "Printing::dataPrintRequested() called not from DataView:" << actionContainer;
        return;
    }

    if (dataView->getModel()->rowCount() == 0)
    {
        notifyError(tr("No data to print."));
        return;
    }

    QPrintDialog* printDialog = new QPrintDialog(MAINWINDOW);
    if (printDialog->exec() != QDialog::Accepted)
        return;

    notifyInfo(tr("Printing data."));

    QString query = dataView->getModel()->getQuery();
    Db* db = dataView->getModel()->getDb();

    printingExport->setPaintDevice(printDialog->printer());

    ExportWorker* worker = new ExportWorker(printingExport, printingConfig, nullptr);
    worker->prepareExportQueryResults(db, query);
    connect(worker, SIGNAL(finished(bool,QIODevice*)), printDialog, SLOT(deleteLater()));
    QThreadPool::globalInstance()->start(worker);
}

void Printing::queryPrintRequested(ExtActionContainer* actionContainer)
{
    EditorWindow* editor = dynamic_cast<EditorWindow*>(actionContainer);
    if (!editor)
    {
        qCritical() << "Printing::queryPrintRequested() called not from EditorWindow:" << actionContainer;
        return;
    }

    QPrintDialog* printDialog = new QPrintDialog(MAINWINDOW);
    if (printDialog->exec() != QDialog::Accepted)
        return;

    notifyInfo(tr("Printing query."));

    QTextDocument* doc = editor->getEditor()->document();
    doc->print(printDialog->printer());
    printDialog->deleteLater();
}
