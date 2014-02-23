#include "completeritemdelegate.h"
#include "completermodel.h"
#include <QPainter>
#include <QIcon>
#include <QApplication>
#include <QVariant>
#include <QDebug>

/*
 * Some of the code in this file comes from Qt5.1, from qcommonstyle.cpp.
 * It uses similar (but different) routines as Qt does for drawing CE_ItemViewItem.
 */

CompleterItemDelegate::CompleterItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void CompleterItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    paintIcon(painter, opt, index);
    paintText(painter, opt, index);
    //QStyledItemDelegate::paint(painter, opt, index);

//    ExpectedToken::Type type = (ExpectedToken::Type)index.data(CompleterModel::TYPE).toInt();
//    QIcon* icon = getIcon(type);

//    QApplication::style()->drawItemPixmap(painter, QRect(1, 1, itemHeight, 20), Qt::AlignVCenter, icon->pixmap(20, 20));
}

QSize CompleterItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (size.height() < 18)
        size.setHeight(18); // at least 1 pixel larger than icons

    return size;
}

void CompleterItemDelegate::paintIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QSize iconSize = icon.availableSizes()[0];

    QIcon::Mode mode = QIcon::Normal;
    if (!(option.state & QStyle::State_Enabled))
        mode = QIcon::Disabled;
    else if (option.state & QStyle::State_Selected)
        mode = QIcon::Selected;

    QRect iconRect = option.rect;
    iconRect.setSize(iconSize + QSize(spacer*2, spacer*2));
    iconRect.setTopLeft(iconRect.topLeft() + QPoint(spacer, spacer));

    QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    icon.paint(painter, iconRect, option.decorationAlignment, mode, state);
}

void CompleterItemDelegate::paintText(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    // Colors
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    QColor prefixColor = option.palette.color(cg, QPalette::Dark);
    QColor valueColor = option.palette.color(cg, QPalette::Text);
    QColor labelColor = option.palette.color(cg, QPalette::Link);
    if (option.state & QStyle::State_Selected)
    {
        prefixColor = option.palette.color(cg, QPalette::HighlightedText);
        valueColor = option.palette.color(cg, QPalette::HighlightedText);
        labelColor = option.palette.color(cg, QPalette::HighlightedText);
    }


    // Using ascent() to measure usual height of the font, excluding anything below baseline.
    int x = option.rect.x() + 20;
    int y = option.rect.y() + option.rect.height() / 2 + option.fontMetrics.ascent() / 2 - spacer;

    painter->setFont(option.font);

    // Getting all data to be painted
    QString prefixValue = index.data(CompleterModel::PREFIX).toString();
    QString value = index.data(CompleterModel::VALUE).toString();
    QString label = index.data(CompleterModel::LABEL).toString();

    // Drawing prefix, value and label
    painter->setPen(prefixColor);
    paintPrefix(painter, option.fontMetrics, x, y, prefixValue);

    painter->setPen(valueColor);
    paintValue(painter, option.fontMetrics, x, y, value);

    painter->setPen(labelColor);
    paintLabel(painter, x, y, label, value.isEmpty());

    painter->restore();
}

void CompleterItemDelegate::paintPrefix(QPainter* painter, const QFontMetrics& metrics, int& x, int y, const QString& text) const
{
    if (text.isNull())
        return;

    QString value = text + ".";
    painter->drawText(QPoint(x, y), value);
    x += metrics.width(value);
}

void CompleterItemDelegate::paintValue(QPainter* painter, const QFontMetrics& metrics, int& x, int y, const QString& text) const
{
    painter->drawText(QPoint(x, y), text);
    x += metrics.width(text);
}

void CompleterItemDelegate::paintLabel(QPainter* painter, int& x, int y, const QString& text, bool emptyValue) const
{
    if (text.isNull())
        return;

    if (!emptyValue) //if the value was empty, there's no reason to move te label right
        x += 10;

    QString label = "(" + text + ")";
    painter->drawText(QPoint(x, y), label);
}
