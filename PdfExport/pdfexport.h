#ifndef PDFEXPORT_H
#define PDFEXPORT_H

#include "pdfexport_global.h"
#include "qtdocexport.h"

class PDFEXPORTSHARED_EXPORT PdfExport : public QtDocExportBase
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pdfexport.json")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        QString getExportConfigFormName() const;
        void validateOptions();
        QString defaultFileExtension() const;

    protected:
        bool writeDoc();

    private:
        static constexpr qreal scaleUpRatio = 12.0;
};

#endif // PDFEXPORT_H
