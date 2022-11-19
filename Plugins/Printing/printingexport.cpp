#include "printingexport.h"
#include "common/unused.h"

QPagedPaintDevice* PrintingExport::createPaintDevice(const QString& documentTitle, bool& takeOwnership)
{
    UNUSED(documentTitle);
    takeOwnership = false;
    return paintDevice;
}

void PrintingExport::setPaintDevice(QPagedPaintDevice* value)
{
    paintDevice = value;
}

bool PrintingExport::init()
{
    lineWidth = 1;
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
    return ExportManager::StandardConfigFlags();
}

ExportManager::ExportModes PrintingExport::getSupportedModes() const
{
    return ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;

}
