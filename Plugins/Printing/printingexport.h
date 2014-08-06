#ifndef PRINTINGEXPORT_H
#define PRINTINGEXPORT_H

#include "printing_global.h"
#include "PdfExport/pdfexport.h"

class PRINTINGSHARED_EXPORT PrintingExport : public PdfExport
{
        Q_OBJECT

    public:
        bool init();
        void deinit();
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportModes getSupportedModes() const;

    protected:
        QPagedPaintDevice* createPaintDevice(const QString& documentTitle);
};

#endif // PRINTINGEXPORT_H
