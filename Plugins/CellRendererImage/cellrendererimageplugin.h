#ifndef CELLRENDERERIMAGEPLUGIN_H
#define CELLRENDERERIMAGEPLUGIN_H

#include "cellrendererimage_global.h"
#include "plugins/genericplugin.h"
#include "datagrid/cellrendererplugin.h"
#include "plugins/uiconfiguredplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(CellRendererImageConfig,
    CFG_CATEGORY(CellRendererImage,
        CFG_ENTRY(int, PreferredMinHeight, 80)
        CFG_ENTRY(int, PreferredMinWidth,  120)
    )
)

class CellRendererImage;

class CELLRENDERERIMAGE_EXPORT CellRendererImagePlugin : public GenericPlugin,
                                                         public CellRendererPlugin,
                                                         public UiConfiguredPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("cellrendererimage.json")

    public:
        QList<DataType> getPreferredTypes();
        QAbstractItemDelegate* createDelegate();
        QSize getPreferredCellSize() const;
        bool init();
        void deinit();
        QString getRendererName() const;
        QString getConfigUiForm() const;
        CfgMain* getMainUiConfig();
        void configDialogOpen();
        void configDialogClosed();

    private:
        CFG_LOCAL_PERSISTABLE(CellRendererImageConfig, cfg)

        CellRendererImage* instance = nullptr;
};

#endif // CELLRENDERERIMAGEPLUGIN_H
