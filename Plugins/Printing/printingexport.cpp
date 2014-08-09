#include "printingexport.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "services/notifymanager.h"

QPagedPaintDevice* PrintingExport::createPaintDevice(const QString& documentTitle)
{
    UNUSED(documentTitle);
    return paintDevice;
}

QPagedPaintDevice* PrintingExport::getPaintDevice() const
{
    return paintDevice;
}

void PrintingExport::setPaintDevice(QPagedPaintDevice* value)
{
    paintDevice = value;
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
