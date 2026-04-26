#include "completersnippetdelegate.h"

#include <QListWidgetItem>
#include <QPainter>

CompleterSnippetDelegate::CompleterSnippetDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

void CompleterSnippetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QStyledItemDelegate::paint(painter, opt, index);
    if (!showHotKeys)
        return;

    QString hotkey = index.data(QListWidgetItem::UserType).toString();
    if (!hotkey.isEmpty())
    {
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        QColor labelColor = option.palette.color(cg, QPalette::Link);
        if (option.state & QStyle::State_Selected)
            labelColor = option.palette.color(cg, QPalette::HighlightedText);

        painter->save();
        painter->setPen(labelColor);
        painter->drawText(opt.rect.adjusted(0, 0, -5, 0), Qt::AlignRight | Qt::AlignVCenter, QString("(%1)").arg(hotkey));
        painter->restore();
    }
}

void CompleterSnippetDelegate::setShowHotkeys(bool newShowHotkeys)
{
    showHotKeys = newShowHotkeys;
}
