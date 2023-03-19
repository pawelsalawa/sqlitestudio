#ifndef ERDEDITORPLUGIN_H
#define ERDEDITORPLUGIN_H

#include "erdeditor_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"

class ExtAction;

class ERDEDITORSHARED_EXPORT ErdEditorPlugin : public GenericPlugin, GeneralPurposePlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("erdeditor.json")

    public:
        bool init();
        void deinit();

    private:
        ExtAction* openErdEditorAction = nullptr;

    private slots:
        void openEditor();
};

#endif // ERDEDITORPLUGIN_H
