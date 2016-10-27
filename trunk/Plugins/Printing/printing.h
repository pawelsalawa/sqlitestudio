#ifndef PRINTING_H
#define PRINTING_H

#include "printing_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"
#include "services/exportmanager.h"

class ExtActionPrototype;
class ExtActionContainer;
class PrintingExport;
class QPrintDialog;

class PRINTINGSHARED_EXPORT Printing : public GenericPlugin, public GeneralPurposePlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("printing.json")

    public:
        bool init();
        void deinit();

    private:
        ExtActionPrototype* separatorAction = nullptr;
        ExtActionPrototype* printDataAction = nullptr;
        ExtActionPrototype* printQueryAction = nullptr;
        PrintingExport* printingExport = nullptr;
        ExportManager::StandardExportConfig* printingConfig = nullptr;

    private slots:
        void dataPrintRequested(ExtActionContainer* actionContainer);
        void queryPrintRequested(ExtActionContainer* actionContainer);
};

#endif // PRINTING_H
