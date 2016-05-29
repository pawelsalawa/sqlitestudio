#include "centerediconitemdelegate.h"
#include <QApplication>
#include <QPainter>

CenteredIconItemDelegate::CenteredIconItemDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

void CenteredIconItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // disable default icon
    opt.icon = QIcon();
    if (opt.features.testFlag(QStyleOptionViewItem::HasDecoration))
        opt.features ^= QStyleOptionViewItem::HasDecoration;

    // draw default item
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, 0);

    const QRect r = option.rect;

    // get pixmap
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QPixmap pix = icon.pixmap(r.size());

    // draw pixmap at center of item
    const QPoint p = QPoint((r.width() - pix.width()) / 2, (r.height() - pix.height()) / 2);
    painter->drawPixmap(r.topLeft() + p, pix);
}
