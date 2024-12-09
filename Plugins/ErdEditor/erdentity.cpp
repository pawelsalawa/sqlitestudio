#include "erdentity.h"
#include "iconmanager.h"
#include "parser/ast/sqlitecreatetable.h"
#include "erdconnection.h"
#include "icon.h"
#include "style.h"
#include <QGraphicsDropShadowEffect>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QDebug>
#include <QSizePolicy>
#include <QSharedPointer>
#include <QPen>
#include <QGraphicsScene>

ErdEntity::ErdEntity(SqliteCreateTable* tableModel) :
    ErdEntity(QSharedPointer<SqliteCreateTable>(tableModel))
{
}

ErdEntity::ErdEntity(QSharedPointer<SqliteCreateTable> tableModel) :
    QGraphicsRectItem(), tableModel(tableModel)
{
    setZValue(10);

    setPen(QPen(STYLE->standardPalette().text().color(), 0.3));
    setBrush(STYLE->standardPalette().window());

    setFlag(QGraphicsItem::ItemIsSelectable, true);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20);
    effect->setOffset(4, 4);
    effect->setColor(QColor(0, 0, 0, 128));
    setGraphicsEffect(effect);

    rebuild();
}

QSharedPointer<SqliteCreateTable> ErdEntity::getTableModel() const
{
    return tableModel;
}

int ErdEntity::rowIndexAt(const QPointF& point)
{
    QPointF localCoords = mapFromScene(point);
    int idx = 0;
    for (Row* row : rows)
    {
        if (row->topRect->contains(localCoords))
            return idx;

        idx++;
    }
    return -1;
}

QRectF ErdEntity::rowRect(int rowIndex)
{
    if (rowIndex > -1 && rowIndex < rows.size())
        return rows[rowIndex]->topRect->boundingRect();

    return QRectF();
}

bool ErdEntity::isClickable()
{
    return true;
}

void ErdEntity::updateConnectionsGeometry()
{
    for (ErdConnection*& conn : connections)
        conn->refreshPosition();
}

void ErdEntity::addConnection(ErdConnection* conn)
{
    connections << conn;
}

void ErdEntity::removeConnection(ErdConnection* conn)
{
    connections.removeOne(conn);
}

QList<ErdConnection*> ErdEntity::getConnections() const
{
    return connections;
}

QString ErdEntity::getTableName() const
{
    return tableModel->table;
}

void ErdEntity::rebuild()
{
    if (!tableModel)
        return;

    int colIdx = 0;
    addTableTitle(tableModel->table);
    for (SqliteCreateTable::Column*& col : tableModel->columns)
        addColumn(col->name, (++colIdx == tableModel->columns.size()));

    qreal iconsWd = 0;
    for (Row* row : rows)
        iconsWd = qMax(iconsWd, row->calcIconsWidth());

    // Total width calculated afterwards, because text width must be added only after total icon column width is known
    qreal totalWd = 0;
    for (Row* row : rows)
        totalWd = qMax(totalWd, row->calcWidth(iconsWd));

    qreal y = 0;
    for (Row* row : rows)
    {
        y += row->updateLayout(iconsWd, totalWd, y);
        row->disableChildSelection();
    }

    setRect(0, 0, totalWd, y);
}

void ErdEntity::addTableTitle(const QString& text)
{
    Row* row = new Row(this);

    QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(row->topRect);
    iconItem->setPixmap(ICONS.TABLE.toQPixmap());
    row->icons << iconItem;

    row->text = new QGraphicsSimpleTextItem(row->topRect);
    row->text->setBrush(STYLE->standardPalette().text());
    row->text->setText(text);

    auto bold = row->text->font();
    bold.setWeight(QFont::ExtraBold);
    row->text->setFont(bold);

    row->bottomLine = new QGraphicsLineItem(row->topRect);
    row->bottomLine->setPen(QPen(STYLE->standardPalette().text().color(), 0.7));

    rows << row;
}

void ErdEntity::addColumn(const QString& text, bool isLast)
{
    Row* row = new Row(this);

    //QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(row->topRect);
    //QPixmap icon = ICONS.TABLE.toQPixmap();
    //iconItem->setPixmap(icon);
    //row->icons << iconItem;

    row->text = new QGraphicsSimpleTextItem(row->topRect);
    row->text->setBrush(STYLE->standardPalette().text());
    row->text->setText(text);

    if (!isLast)
    {
        row->bottomLine = new QGraphicsLineItem(row->topRect);
        row->bottomLine->setPen(QPen(STYLE->standardPalette().text().color(), 0.3));
    }

    rows << row;
}

void ErdEntity::modelUpdated()
{
    for (Row* row : rows)
    {
        scene()->removeItem(row->topRect);
        delete row;
    }
    rows.clear();
    rebuild();
}

ErdEntity::Row::Row(QGraphicsItem* parent)
{
    topRect = new QGraphicsRectItem(parent);
    topRect->setPen(QPen(QColorConstants::Transparent, 0));
}

ErdEntity::Row::~Row()
{
    delete topRect;
}

qreal ErdEntity::Row::height() const
{
    qreal hg = 0;
    for (QGraphicsItem* icon : icons)
        hg = qMax(hg, icon->boundingRect().height());

    hg = qMax(hg, text->boundingRect().height());
    return hg;
}

qreal ErdEntity::Row::calcIconsWidth() const
{
    qreal iconsWd = 0;
    for (QGraphicsItem* iconItem : icons)
        iconsWd += iconItem->boundingRect().width();

    return iconsWd + (icons.size() - 1) * ICON_GAP;
}

qreal ErdEntity::Row::calcWidth(qreal iconColumn) const
{
    qreal wd = CELL_PADDING;
    wd += iconColumn + TEXT_GAP;
    wd += text->boundingRect().width();
    return wd + CELL_PADDING;
}

qreal ErdEntity::Row::updateLayout(qreal iconColumn, qreal globalWidth, qreal globalY)
{
    // Calculate max height of all items + paddings
    qreal hg = 0;
    for (QGraphicsItem* iconItem : icons)
        hg = qMax(iconItem->boundingRect().height(), hg);

    hg = qMax(text->boundingRect().height(), hg);
    hg += CELL_PADDING * 2;

    // Place items starting from left, so icons first.
    // Since we have pre-calculated max width of icons in all rows, we need to align icons to the right here
    qreal x = CELL_PADDING + iconColumn;

    for (QGraphicsItem* iconItem : icons)
    {
        auto iconRect = iconItem->boundingRect();
        qreal iconY = hg / 2 - iconRect.height() / 2;
        qreal iconWd = iconRect.width();
        x -= iconWd;
        iconItem->setPos(x, iconY);
        x -= ICON_GAP;
    }

    // Reset to end of icons and move on with the rest
    x = CELL_PADDING + iconColumn + TEXT_GAP;

    // Text item
    auto textRect = text->boundingRect();
    qreal textY = hg / 2 - textRect.height() / 2;
    text->setPos(x, textY);
    x += textRect.width();

    // Rightpadding
    x += CELL_PADDING;

    // Bottom line
    if (bottomLine)
        bottomLine->setLine(1, hg - 1, globalWidth - 1, hg - 1);

    // Knowing all dimensions, let's update top-level rect of this row
    topRect->setPos(0, globalY);
    topRect->setRect(0, globalY, globalWidth, hg);

    return hg;
}

void ErdEntity::Row::disableChildSelection()
{
    topRect->setFlag(QGraphicsItem::ItemIsSelectable, false);
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);
    for (QGraphicsItem* iconItem : icons)
        iconItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

    if (bottomLine)
        bottomLine->setFlag(QGraphicsItem::ItemIsSelectable, false);
}
