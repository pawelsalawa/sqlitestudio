#include "printingexport.h"
#include "common/unused.h"
#include "mainwindow.h"
#include <QPrinter>
#include <QPrintDialog>

QPagedPaintDevice* PrintingExport::createPaintDevice(const QString& documentTitle)
{
    UNUSED(documentTitle);

    QPrinter* printer = new QPrinter();
    QPrintDialog dialog(printer, MAINWINDOW);
    if (dialog.exec() == QDialog::Accepted)
        return printer;

    return nullptr;
}

bool PrintingExport::init()
{
    return PdfExport::init();
}

void PrintingExport::deinit()
{
}

QString PrintingExport::getFormatName() const
{
    return tr("Printing");
}

ExportManager::StandardConfigFlags PrintingExport::standardOptionsToEnable() const
{
    return 0;
}

ExportManager::ExportModes PrintingExport::getSupportedModes() const
{
    return ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;

}
