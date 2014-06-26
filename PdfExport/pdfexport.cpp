#include "pdfexport.h"
#include <QTextDocument>
#include <QPainter>
#include <QPdfWriter>

QString PdfExport::getFormatName() const
{
    return "PDF";
}

ExportManager::StandardConfigFlags PdfExport::standardOptionsToEnable() const
{
    return 0;
}

QString PdfExport::getExportConfigFormName() const
{
    return QString();
}

void PdfExport::validateOptions()
{
}

QString PdfExport::defaultFileExtension() const
{
    return "pdf";
}

bool PdfExport::writeDoc()
{
    QPdfWriter pdfWriter(output);
    pdfWriter.setCreator(QString("SQLiteStudio v%1").arg(SQLITESTUDIO->getVersionString()));
    pdfWriter.setPageSize(QPagedPaintDevice::A4);
    QPainter painter(&pdfWriter);
    painter.scale(scaleUpRatio, scaleUpRatio);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const static qreal PT_MM = 25.4/72.0; // point-to-mm size ration, used by Qt
    getDocument()->setPageSize(pdfWriter.pageSizeMM()/PT_MM);

    getDocument()->drawContents(&painter);
    return true;
}

