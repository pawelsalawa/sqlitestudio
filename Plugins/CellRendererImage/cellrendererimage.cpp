#include "cellrendererimage.h"
#include "datagrid/sqlquerymodel.h"
#include <QApplication>
#include <QBuffer>
#include <QImageReader>
#include <QPainter>
#include <QDebug>

CellRendererImage::CellRendererImage(CfgTypedEntry<int>* widthCfg, CfgTypedEntry<int>* heightCfg, QObject* parent) :
    SqlQueryItemDelegate(parent), widthCfg(widthCfg), heightCfg(heightCfg)
{
}

void CellRendererImage::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    const QVariant var = index.data(Qt::EditRole);
    if (!var.canConvert<QByteArray>())
    {
        SqlQueryItemDelegate::paint(painter, option, index);
        return;
    }

    QByteArray imageData = var.toByteArray();
    if (imageData.isEmpty())
        return;

    QBuffer buf(&imageData);
    QImageReader ir(&buf);
    if (!ir.canRead())
    {
        SqlQueryItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QImage image;
    if (!image.loadFromData(imageData))
    {
        SqlQueryItemDelegate::paint(painter, option, index);
        return;
    }

    QRect contentRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    if (contentRect.isEmpty())
        return;

    QImage scaled = image.scaled(contentRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPoint topLeft(
        contentRect.x() + (contentRect.width()  - scaled.width())  / 2,
        contentRect.y() + (contentRect.height() - scaled.height()) / 2
    );

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(topLeft, scaled);
    painter->restore();
}

QSize CellRendererImage::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(widthCfg->get(), heightCfg->get());
}

QString CellRendererImage::displayText(const QVariant& value, const QLocale& locale) const
{
    if (!value.canConvert<QByteArray>())
        return SqlQueryItemDelegate::displayText(value, locale);

    QByteArray imageData = value.toByteArray();
    if (imageData.isEmpty())
        return SqlQueryItemDelegate::displayText(value, locale);

    QBuffer buf(&imageData);
    QImageReader ir(&buf);
    if (!ir.canRead())
        return SqlQueryItemDelegate::displayText(value, locale);

    Q_UNUSED(value)
    Q_UNUSED(locale)
    return QString();
}

