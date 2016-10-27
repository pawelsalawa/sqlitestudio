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

        void setPaintDevice(QPagedPaintDevice* value);

    protected:
        QPagedPaintDevice* createPaintDevice(const QString& documentTitle, bool& takeOwnership);

    private:
        QPagedPaintDevice* paintDevice = nullptr;
};

#endif // PRINTINGEXPORT_H
