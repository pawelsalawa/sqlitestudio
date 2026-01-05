#ifndef ERDEDITORPLUGIN_H
#define ERDEDITORPLUGIN_H

#include "erdeditor_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/uiconfiguredplugin.h"
#include "config_builder.h"
#include "scene/erdarrowitem.h"

CFG_CATEGORIES(ErdConfig,
    CFG_CATEGORY(Erd,
        CFG_ENTRY(ErdArrowItem::Type, ArrowType,     ErdArrowItem::CURVY)
        CFG_ENTRY(long,               MaxTableLimit, 200)
        CFG_ENTRY(bool,               DragBySpace,   false)
    )
)

class QAction;

class ERDEDITORSHARED_EXPORT ErdEditorPlugin : public GenericPlugin, GeneralPurposePlugin, public UiConfiguredPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("ErdEditor.json")

    public:
        bool init();
        void deinit();
        QString getConfigUiForm() const;
        CfgMain* getMainUiConfig();
        void configDialogOpen();
        void configDialogClosed();

        CFG_LOCAL_PERSISTABLE(ErdConfig, cfg)

        static ErdEditorPlugin* instance;

    private:
        QAction* openErdEditorAction = nullptr;

    private slots:
        void openEditor();
};

#define CFG_ERD ErdEditorPlugin::instance->cfg

#endif // ERDEDITORPLUGIN_H
