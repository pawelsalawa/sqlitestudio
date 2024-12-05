#include "erdentity.h"
#include "iconmanager.h"
#include "parser/ast/sqlitecreatetable.h"
#include "erdconnection.h"
#include "icon.h"
#include <QGraphicsDropShadowEffect>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QSizePolicy>
#include <QSharedPointer>

ErdEntity::ErdEntity(SqliteCreateTable* tableModel) :
    ErdEntity(QSharedPointer<SqliteCreateTable>(tableModel))
{
}

ErdEntity::ErdEntity(QSharedPointer<SqliteCreateTable> tableModel) :
    QGraphicsProxyWidget(), tableModel(tableModel)
{
    setZValue(10);

    frame = new QFrame();
    frame->setStyleSheet(".QFrame {border: 1px solid rgba(128, 128, 128, 0.5);}");
    setWidget(frame);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20);
    effect->setOffset(4, 4);
    effect->setColor(QColor(0, 0, 0, 128));
    setGraphicsEffect(effect);

    layout = new QGridLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    frame->setLayout(layout);

    rebuild();
}

QSharedPointer<SqliteCreateTable> ErdEntity::getTableModel() const
{
    return tableModel;
}

int ErdEntity::rowIndexAt(const QPointF& point)
{
    QPoint localCoords = mapFromScene(point).toPoint();

    for (int row = 0, rows = layout->rowCount(); row < rows; row++)
    {
        if (rowRect(row).contains(localCoords))
            return row;
    }
    return -1;
}

QRectF ErdEntity::rowRect(int rowIndex)
{
    QRectF rect = layout->cellRect(rowIndex, 1);
    rect.setLeft(0);
    rect.setRight(rect.width());
    return rect;
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

void ErdEntity::clearLayout()
{
    QLayoutItem* child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }
}

void ErdEntity::rebuild()
{
    if (!tableModel)
        return;

    addTableTitle(tableModel->table);
    for (SqliteCreateTable::Column*& col : tableModel->columns)
    {
        addColumn(col->name);
    }
}

void ErdEntity::addColumn(const QString& text)
{
    QLayoutItem* item = setLabel(text);
    item->widget()->setStyleSheet("QLabel {border-top: 1px solid rgba(128, 128, 128, 0.5); padding: 3px;}");

    QLayoutItem* iconItem = setBlankIcon();
    iconItem->widget()->setStyleSheet("QLabel {border-top: 1px solid rgba(128, 128, 128, 0.5); padding: 3px;}");

    currRow++;
}

void ErdEntity::addTableTitle(const QString& text)
{
    QLayoutItem* item = setLabel(text);
    item->widget()->setStyleSheet("QLabel {font-weight: bold; padding: 8px 3px;}");

    QLayoutItem* iconItem = setIcon(ICONS.TABLE);
    iconItem->widget()->setStyleSheet("QLabel {font-weight: bold; padding: 8px 3px;}");

    currRow++;
}

QLayoutItem* ErdEntity::setIcon(Icon* icon)
{
    QLabel* label = new QLabel();
    label->setPixmap(icon->toQPixmap());
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    layout->addWidget(label, currRow, 0);
    return layout->itemAtPosition(currRow, 0);
}

QLayoutItem* ErdEntity::setBlankIcon()
{
    QLabel* label = new QLabel();
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    layout->addWidget(label, currRow, 0);
    return layout->itemAtPosition(currRow, 0);
}

QLayoutItem* ErdEntity::setLabel(const QString& text)
{
    QLabel* label = new QLabel(text);
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    layout->addWidget(label, currRow, 1);
    return layout->itemAtPosition(currRow, 1);
}


void ErdEntity::modelUpdated()
{
    clearLayout();
    rebuild();
}
