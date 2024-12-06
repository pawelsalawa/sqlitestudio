#ifndef ERDEDITORPLUGIN_H
#define ERDEDITORPLUGIN_H

#include "erdeditor_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"
#include "config_builder.h"
#include "erdarrowitem.h"

CFG_CATEGORIES(ErdConfig,
     CFG_CATEGORY(Erd,
         CFG_ENTRY(ErdArrowItem::Type, ArrowType, ErdArrowItem::CURVY)
     )
)

class QAction;

class ERDEDITORSHARED_EXPORT ErdEditorPlugin : public GenericPlugin, GeneralPurposePlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("erdeditor.json")

    public:
        bool init();
        void deinit();

    private:
        QAction* openErdEditorAction = nullptr;

    private slots:
        void openEditor();
};

#define CFG_ERD CFG_INSTANCE(ErdConfig)

#endif // ERDEDITORPLUGIN_H
