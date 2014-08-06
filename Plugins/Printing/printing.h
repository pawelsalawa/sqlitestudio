#ifndef PRINTING_H
#define PRINTING_H

#include "printing_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"

class QAction;

class PRINTINGSHARED_EXPORT Printing : public GenericPlugin, public GeneralPurposePlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("printing.json")

    public:
        bool init();
        void deinit();

    private:
        QAction* printResultsAction = nullptr;
        QAction* printTableAction = nullptr;
        QAction* printDatabaseAction = nullptr;
};

#endif // PRINTING_H
