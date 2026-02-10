#include "cellrendererimage.h"

CellRendererImage::CellRendererImage(QObject* parent) :
    SqlQueryItemDelegate(parent)
{
}

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
        instance = new CellRendererImage();

    return instance;
}

void CellRendererImagePlugin::deinit()
{
    delete instance;
    instance = nullptr;
}

QString CellRendererImagePlugin::getRendererName() const
{
    return tr("Image", "cell preview renderer name");
}
