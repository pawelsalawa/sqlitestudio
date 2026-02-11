#ifndef CELLRENDERERIMAGE_H
#define CELLRENDERERIMAGE_H

#include "datagrid/sqlqueryitemdelegate.h"
#include "config_builder/cfgentry.h"

class CellRendererImage : public SqlQueryItemDelegate
{
    Q_OBJECT

    public:
        explicit CellRendererImage(CfgTypedEntry<int>* widthCfg, CfgTypedEntry<int>* heightCfg, QObject *parent = 0);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QString	displayText(const QVariant & value, const QLocale & locale) const;

    private:
         CfgTypedEntry<int>* widthCfg = nullptr;
         CfgTypedEntry<int>* heightCfg = nullptr;
};


#endif // CELLRENDERERIMAGE_H
