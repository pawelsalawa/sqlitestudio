#include "cellrendererimageplugin.h"
#include "cellrendererimage.h"

QList<DataType> CellRendererImagePlugin::getPreferredTypes()
{
    return {
        DataType("PICTURE"),
        DataType("PIC"),
        DataType("IMAGE"),
        DataType("IMG")
    };
}

QAbstractItemDelegate* CellRendererImagePlugin::createDelegate()
{
    if (!instance)
        instance = new CellRendererImage(&(cfg.CellRendererImage.PreferredMinWidth), &(cfg.CellRendererImage.PreferredMinHeight));

    return instance;
}

QSize CellRendererImagePlugin::getPreferredCellSize() const
{
    return QSize(cfg.CellRendererImage.PreferredMinWidth.get(), cfg.CellRendererImage.PreferredMinHeight.get());
}

bool CellRendererImagePlugin::init()
{
    SQLS_INIT_RESOURCE(cellrendererimage);
    return true;
}

void CellRendererImagePlugin::deinit()
{
    delete instance;
    instance = nullptr;
    SQLS_CLEANUP_RESOURCE(cellrendererimage);
}

QString CellRendererImagePlugin::getRendererName() const
{
    return tr("Image", "cell preview renderer name");
}

QString CellRendererImagePlugin::getConfigUiForm() const
{
    return "CellRendererImage";
}

CfgMain* CellRendererImagePlugin::getMainUiConfig()
{
    return &cfg;
}

void CellRendererImagePlugin::configDialogOpen()
{
}

void CellRendererImagePlugin::configDialogClosed()
{
}
