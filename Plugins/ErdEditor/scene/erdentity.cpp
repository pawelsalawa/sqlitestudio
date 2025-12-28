#include "erdentity.h"
#include "iconmanager.h"
#include "erdconnection.h"
#include "icon.h"
#include "style.h"
#include "common/unused.h"
#include "common/global.h"
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
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QGraphicsProxyWidget>
#include <QLineEdit>

ErdEntity::ErdEntity(SqliteCreateTable* tableModel) :
    ErdEntity(SqliteCreateTablePtr(tableModel))
{
}

ErdEntity::ErdEntity(const SqliteCreateTablePtr& tableModel) :
    QObject(), QGraphicsRectItem(), tableModel(tableModel)
{
    setZValue(10);

    setPen(QPen(STYLE->standardPalette().text().color(), 0.3));
    setBrush(STYLE->standardPalette().window());

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20);
    effect->setOffset(4, 4);
    effect->setColor(QColor(0, 0, 0, 128));
    setGraphicsEffect(effect);

    rebuild();
}

SqliteCreateTablePtr ErdEntity::getTableModel() const
{
    return tableModel;
}

SqliteStatement* ErdEntity::getStatementAtRowIndex(int rowIdx) const
{
    if (rowIdx < 0 || rowIdx >= rows.size())
        return nullptr;

    return rows[rowIdx]->sqliteStatement;
}

void ErdEntity::setTableModel(const SqliteCreateTablePtr& tableModel)
{
    this->tableModel = tableModel;
}

int ErdEntity::rowIndexAt(const QPointF& point)
{
    QPointF localCoords = mapFromScene(point);
    int idx = 0;
    for (Row*& row : rows)
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

void ErdEntity::clearConnections()
{
    for (ErdConnection*& conn : connections)
        delete conn;

    if (!connections.isEmpty())
        qCritical() << "Connections of entity is not empty after clearing! Current count is:" << connections.size();
}

QList<ErdConnection*> ErdEntity::getConnections() const
{
    return connections;
}

QList<ErdConnection*> ErdEntity::getOwningConnections() const
{
    return connections | FILTER(conn, {return conn->getStartEntity() == this;});
}

QString ErdEntity::getTableName() const
{
    return tableModel->table;
}

void ErdEntity::updateConnectionIndexes()
{
    if (rows.size() <= 1)
        return; // only the table header

    QHash<int, QList<ErdConnection*>> connsByRowIdx = connections | GROUP_BY(conn,
    {
        if (conn->getStartEntity() == this)
            return conn->getStartEntityRow();
        else
            return conn->getEndEntityRow();
    });

    QList<ErdConnection*> radialSortedConnections;
    int minIdx = 1;
    int maxIdx = rows.size()-1;
    while (minIdx < maxIdx)
    {
        radialSortedConnections += connsByRowIdx[minIdx];
        minIdx++;
        if (minIdx < maxIdx)
        {
            radialSortedConnections += connsByRowIdx[maxIdx];
            maxIdx--;
        }
    }

    int connectionInEntityIndex = 0;
    for (ErdConnection*& conn : radialSortedConnections)
    {
        if (conn->getStartEntity() == this)
            conn->setIndexInStartEntity(connectionInEntityIndex++);
        else
            conn->setIndexInEndEntity(connectionInEntityIndex++);
    }
}

void ErdEntity::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    UNUSED(option);
    UNUSED(widget);
    int radius = 4;
    painter->setBrush(brush());
    painter->setPen(pen());
    QRectF rect = boundingRect();
    painter->drawRoundedRect(rect, radius, radius);

    if (isSelected())
    {
        QPen outlinePen;
        outlinePen.setColor(STYLE->standardPalette().highlight().color());
        outlinePen.setStyle(Qt::DotLine);
        outlinePen.setWidth(2);
        painter->setPen(outlinePen);
        painter->drawRoundedRect(rect, radius, radius);
    }
}

void ErdEntity::rebuild()
{
    if (!tableModel)
        return;

    int colIdx = 0;
    addTableTitle();
    for (SqliteCreateTable::Column*& col : tableModel->columns)
        addColumn(col, (++colIdx == tableModel->columns.size()));

    qreal iconsWd = 0;
    for (Row*& row : rows)
        iconsWd = qMax(iconsWd, row->calcIconsWidth());

    qreal nameWd = 0;
    for (Row*& row : rows.mid(1)) // skip header, as it has no distinct name & datatype columns
        nameWd = qMax(nameWd, row->calcNameWidth());

    // Total width calculated afterwards, because text width must be added only after total icon column width is known
    qreal totalWd = 0;
    for (Row*& row : rows)
        totalWd = qMax(totalWd, row->calcWidth(iconsWd, nameWd));

    qreal y = 0;
    for (Row*& row : rows)
        y += row->updateLayout(iconsWd, nameWd, totalWd, y);

    disableChildSelection(this);
    // enableChildFocusing(this);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    setRect(0, 0, totalWd, y);

    // cornerIcon = new QGraphicsPixmapItem(this);
    // cornerIcon->setPixmap(ICONS.INDICATOR_WARN.toQPixmap(12));
    // cornerIcon->setToolTip(QObject::tr(""));
    // cornerIcon->setPos(-6, -6);
}

void ErdEntity::addTableTitle()
{
    Row* row = new Row(this);
    row->isHeader = true;
    row->sqliteStatement = tableModel.data();

    QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(row->topRect);
    iconItem->setPixmap(ICONS.TABLE.toQPixmap());
    row->icons << iconItem;

    row->text = new QGraphicsSimpleTextItem(row->topRect);
    row->text->setBrush(STYLE->standardPalette().text());
    row->text->setText(tableModel->table);

    auto bold = row->text->font();
    bold.setWeight(QFont::ExtraBold);
    bold.setPointSizeF(bold.pointSizeF() * 1.4);
    row->text->setFont(bold);

    row->bottomLine = new QGraphicsLineItem(row->topRect);
    row->bottomLine->setPen(QPen(STYLE->standardPalette().text().color(), 0.7));

    rows << row;
}

bool ErdEntity::isExistingTable() const
{
    return existingTable;
}

void ErdEntity::setExistingTable(bool newExistingTable)
{
    existingTable = newExistingTable;
}

bool ErdEntity::edit(const QPointF& point)
{
    int idx = rowIndexAt(point);
    if (idx < 0)
        return false;

    editRow(idx);
    return true;
}

void ErdEntity::editName()
{
    editRow(0);
}

void ErdEntity::editRow(int rowIdx)
{
    Row* row = rows[rowIdx];
    QGraphicsProxyWidget* proxy = new QGraphicsProxyWidget(row->topRect);

    QLineEdit* edit = new QLineEdit();
    edit->setText(row->text->text());
    proxy->setWidget(edit);
    proxy->setGeometry(row->topRect->boundingRect());
    edit->selectAll();
    edit->setFocus();
}

void ErdEntity::addColumn(SqliteCreateTable::Column* column, bool isLast)
{
    Row* row = new Row(this);
    row->sqliteStatement = column;

    for (SqliteCreateTable::Column::Constraint*& constr : column->constraints)
    {
        QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(row->topRect);
        iconItem->setToolTip(constr->detokenize().trimmed());
        Icon* icon = nullptr;
        switch (constr->type)
        {
            case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
                icon = &(ICONS.CONSTRAINT_PRIMARY_KEY);
                break;
            case SqliteCreateTable::Column::Constraint::NOT_NULL:
                icon = &(ICONS.CONSTRAINT_NOT_NULL);
                break;
            case SqliteCreateTable::Column::Constraint::UNIQUE:
                icon = &(ICONS.CONSTRAINT_UNIQUE);
                break;
            case SqliteCreateTable::Column::Constraint::CHECK:
                icon = &(ICONS.CONSTRAINT_CHECK);
                break;
            case SqliteCreateTable::Column::Constraint::DEFAULT:
                icon = &(ICONS.CONSTRAINT_DEFAULT);
                break;
            case SqliteCreateTable::Column::Constraint::GENERATED:
                icon = &(ICONS.CONSTRAINT_GENERATED);
                break;
            case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
                icon = &(ICONS.CONSTRAINT_FOREIGN_KEY);
                break;
            case SqliteCreateTable::Column::Constraint::COLLATE:
                icon = &(ICONS.CONSTRAINT_COLLATION);
                break;
            case SqliteCreateTable::Column::Constraint::NULL_:
            case SqliteCreateTable::Column::Constraint::NAME_ONLY:
            case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
                break;
        }

        if (icon)
        {
            iconItem->setPixmap(icon->toQPixmap());
            row->icons << iconItem;
        }
    }

    row->text = new QGraphicsSimpleTextItem(row->topRect);
    row->text->setBrush(STYLE->standardPalette().text());
    row->text->setText(column->name);

    auto bold = row->text->font();
    bold.setWeight(QFont::Bold);
    row->text->setFont(bold);

    if (column->type)
    {
        row->datatype = new QGraphicsSimpleTextItem(row->topRect);
        row->datatype->setBrush(STYLE->standardPalette().text());
        row->datatype->setText(column->type->detokenize().trimmed());
    }

    if (!isLast)
    {
        row->bottomLine = new QGraphicsLineItem(row->topRect);
        row->bottomLine->setPen(QPen(STYLE->standardPalette().text().color(), 0.3));
    }

    rows << row;
}

void ErdEntity::modelUpdated()
{
    if (cornerIcon)
    {
        scene()->removeItem(cornerIcon);
        delete cornerIcon;
        cornerIcon = nullptr;
    }

    for (Row*& row : rows)
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

qreal ErdEntity::Row::calcNameWidth() const
{
    return text->boundingRect().width();
}

qreal ErdEntity::Row::calcWidth(qreal iconColumn, qreal nameColumn) const
{
    qreal wd = CELL_PADDING;
    wd += iconColumn + TEXT_GAP;

    if (isHeader)
    {
        wd += text->boundingRect().width();
    }
    else
    {
        wd += nameColumn;
        if (datatype)
            wd += TEXT_GAP + datatype->boundingRect().width();
    }

    return wd + CELL_PADDING;
}

qreal ErdEntity::Row::updateLayout(qreal iconColumn, qreal nameColumn, qreal globalWidth, qreal globalY)
{
    // Calculate max height of all items + paddings
    qreal hg = 0;
    for (QGraphicsItem*& iconItem : icons)
        hg = qMax(iconItem->boundingRect().height(), hg);

    hg = qMax(text->boundingRect().height(), hg);
    if (datatype)
        hg = qMax(datatype->boundingRect().height(), hg);

    hg += CELL_PADDING * 2;

    if (isHeader)
        hg += CELL_PADDING * 2;

    // Place items starting from left, so icons first.
    // Since we have pre-calculated max width of icons in all rows, we need to align icons to the right here
    qreal x = CELL_PADDING + iconColumn;

    for (QGraphicsItem*& iconItem : icons)
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

    // Datatype item
    if (datatype)
    {
        auto datatypeRect = datatype->boundingRect();
        qreal datatypeY = hg / 2 - datatypeRect.height() / 2;
        x = CELL_PADDING + iconColumn + TEXT_GAP + nameColumn + TEXT_GAP;
        datatype->setPos(x, datatypeY);
        x += datatypeRect.width();
    }

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

void ErdEntity::disableChildSelection(QGraphicsItem* parent)
{
    for (QGraphicsItem* child : parent->childItems())
    {
        child->setFlag(QGraphicsItem::ItemIsSelectable, false);
        disableChildSelection(child);
    }
}

void ErdEntity::enableChildFocusing(QGraphicsItem* parent)
{
    for (QGraphicsItem* child : parent->childItems())
    {
        child->setFlag(QGraphicsItem::ItemIsFocusable, true);
        enableChildFocusing(child);
    }
}
