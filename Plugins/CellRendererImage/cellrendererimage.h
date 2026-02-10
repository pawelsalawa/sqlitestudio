#ifndef CELLRENDERERIMAGE_H
#define CELLRENDERERIMAGE_H

#include "cellrendererimage_global.h"
#include "plugins/genericplugin.h"
#include "datagrid/cellrendererplugin.h"
#include "datagrid/sqlqueryitemdelegate.h"

class CellRendererImage : public SqlQueryItemDelegate
{
    public:
        explicit CellRendererImage(QObject *parent = 0);
};

class CELLRENDERERIMAGE_EXPORT CellRendererImagePlugin : public GenericPlugin, public CellRendererPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN("cellrendererimage.json")

    public:
        QList<DataType> getPreferredTypes();
        QAbstractItemDelegate* createDelegate();
        void deinit();
        QString getRendererName() const;

    private:
        CellRendererImage* instance = nullptr;
};

#endif // CELLRENDERERIMAGE_H
