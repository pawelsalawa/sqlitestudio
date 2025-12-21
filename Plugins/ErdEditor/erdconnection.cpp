#include "erdconnection.h"
#include "erdlinearrowitem.h"
#include "erdcurvyarrowitem.h"
#include "erdentity.h"
#include "erdscene.h"
#include "erdeditorplugin.h"
#include "erdchangeentity.h"
#include "db/db.h"
#include "db/chainexecutor.h"
#include <QDebug>
#include <QGraphicsScene>

ErdConnection::ErdConnection(ErdEntity* startEntity, const QPointF& endPos, ErdArrowItem::Type arrowType) :
    startEntity(startEntity), volatileEndPosition(endPos)
{
    startEntityRow = startEntity->rowIndexAt(endPos);
    arrow = ErdArrowItem::create(arrowType);
    arrow->setFlag(QGraphicsItem::ItemIsMovable, false);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
    arrow->setFlag(QGraphicsItem::ItemIsFocusable, true);
    refreshPosition();
    startEntity->addConnection(this);
}

ErdConnection::ErdConnection(ErdEntity* startEntity, int startRow, ErdEntity* endEntity, int endRow, ErdArrowItem::Type arrowType) :
    startEntity(startEntity), endEntity(endEntity), startEntityRow(startRow), endEntityRow(endRow)
{
    arrow = ErdArrowItem::create(arrowType);
    refreshPosition();
    arrow->setFlag(QGraphicsItem::ItemIsMovable, false);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
    arrow->setFlag(QGraphicsItem::ItemIsFocusable, true);
    startEntity->addConnection(this);
    endEntity->addConnection(this);
}

ErdConnection::~ErdConnection()
{
    startEntity->removeConnection(this);
    if (endEntity)
        endEntity->removeConnection(this);

    delete arrow;
}

void ErdConnection::addToScene(ErdScene* scene)
{
    this->scene = scene;
    scene->addItem(arrow);
}

void ErdConnection::updatePosition(const QPointF& endPos)
{
    volatileEndPosition = endPos;
    refreshPosition();
}

void ErdConnection::finalizeConnection(ErdEntity* entity, const QPointF& endPos)
{
    endEntity = entity;
    endEntityRow = entity->rowIndexAt(endPos);
    endEntity->addConnection(this);
    startEntity->updateConnectionIndexes();
    endEntity->updateConnectionIndexes();
    refreshPosition();
    commitFinalizationChange();
}

bool ErdConnection::isFinalized() const
{
    return endEntity != nullptr;
}

void ErdConnection::refreshPosition()
{
    QPointF srcPos;
    QPointF trgPos;
    ErdArrowItem::Side startEntitySide = ErdArrowItem::UNDEFINED;
    ErdArrowItem::Side endEntitySide = ErdArrowItem::UNDEFINED;
    if (endEntity)
    {
        QPointF endPos = endEntity->pos();
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, endPos, startEntitySide);
        trgPos = findThisPosAgainstOther(endEntity, endEntityRow, srcPos, endEntitySide);
    }
    else
    {
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, volatileEndPosition, startEntitySide);
        trgPos = volatileEndPosition;
    }
    arrow->setPoints(QLineF(srcPos, trgPos), startEntitySide, endEntitySide);
}

QPointF ErdConnection::findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition, ErdArrowItem::Side& entitySide)
{
    QPointF entityPos = thisEntity->pos();
    QRectF rowRect = (thisRow > -1) ? thisEntity->rowRect(thisRow) : QRectF(0, 0, 0, 0);
    QRectF entityRect = thisEntity->rect();
    entitySide = ErdArrowItem::LEFT;

    QPointF pos = entityPos + rowRect.topLeft() + QPointF(0, rowRect.height() / 2);
    if (otherPosition.x() > (entityPos.x() + (entityRect.width()) / 2))
    {
        pos.rx() += entityRect.width();
        entitySide = ErdArrowItem::RIGHT;
    }

    return pos;
}

void ErdConnection::commitFinalizationChange()
{
    SqliteCreateTablePtr originalCreateTable = startEntity->getTableModel();
    SqliteCreateTablePtr createTable = SqliteCreateTablePtr(originalCreateTable->typeClone<SqliteCreateTable>());

    SqliteCreateTable::Column* startEntityColumn = getStartEntityColumn();
    if (!startEntityColumn)
    {
        qCritical() << "startEntityColumn is null while trying to commit ERD connection.";
        return;
    }

    SqliteCreateTable::Column* endEntityColumn = getEndEntityColumn();
    if (!endEntityColumn)
    {
        qCritical() << "endEntityColumn is null while trying to commit ERD connection.";
        return;
    }

    SqliteCreateTable::Column* column = createTable->columns | FIND_FIRST(col,
    {
        return col->name.compare(startEntityColumn->name, Qt::CaseInsensitive) == 0;
    });

    SqliteCreateTable::Column::Constraint* fk = new SqliteCreateTable::Column::Constraint();
    fk->setParent(column);
    column->constraints << fk;

    SqliteIndexedColumn* idxCol = new SqliteIndexedColumn(endEntityColumn->name);
    fk->initFk(endEntity->getTableName(), {idxCol}, {});

    createTable->rebuildTokens(); // needed for TableModifier when executing changes
    //qDebug() << "dll:" << createTable->detokenize();

    QString desc = QObject::tr("Create relationship between \"%1\" and \"%2\".").arg(createTable->table, fk->foreignKey->foreignTable);
    ErdChange* change = new ErdChangeEntity(scene->getDb(), originalCreateTable, createTable, desc);

    ChainExecutor* ddlExecutor = new ChainExecutor();
    ddlExecutor->setAsync(false);
    ddlExecutor->setDb(scene->getDb());
    ddlExecutor->setQueries(change->toDdl());
    ddlExecutor->setDisableForeignKeys(true);
    ddlExecutor->setDisableObjectDropsDetection(true);
    ddlExecutor->exec();

    if (ddlExecutor->getSuccessfulExecution())
        scene->notify(change);
    else
        qDebug() << "ErdConnection failed to execute DDL change for finalized connection. Errors:" << ddlExecutor->getErrorsMessages();

    delete ddlExecutor;
}

bool ErdConnection::isTableLevelFk() const
{
    return tableLevelFk;
}

void ErdConnection::setTableLevelFk(bool value)
{
    tableLevelFk = value;
}

ErdEntity* ErdConnection::getEndEntity() const
{
    return endEntity;
}

int ErdConnection::getStartEntityRow() const
{
    return startEntityRow;
}

int ErdConnection::getEndEntityRow() const
{
    return endEntityRow;
}

SqliteCreateTable::Column* ErdConnection::getStartEntityColumn() const
{
    if (!startEntity || startEntityRow < 0)
        return nullptr;

    return dynamic_cast<SqliteCreateTable::Column*>(startEntity->getStatementAtRowIndex(startEntityRow));
}

SqliteCreateTable::Column* ErdConnection::getEndEntityColumn() const
{
    if (!endEntity || endEntityRow < 0)
        return nullptr;

    return dynamic_cast<SqliteCreateTable::Column*>(endEntity->getStatementAtRowIndex(endEntityRow));
}

void ErdConnection::setArrowType(ErdArrowItem::Type arrowType)
{
    scene->removeItem(arrow);
    delete arrow;

    arrow = ErdArrowItem::create(arrowType);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
    arrow->setFlag(QGraphicsItem::ItemIsMovable, false);
    arrow->setFlag(QGraphicsItem::ItemIsFocusable, true);
    arrow->setArrowIndexInStartEntity(indexInStartEntity);
    arrow->setArrowIndexInEndEntity(indexInEndEntity);
    scene->addItem(arrow);
    refreshPosition();
}

void ErdConnection::select(bool changeFocusToo)
{
    arrow->setSelected(true);
    if (changeFocusToo)
        arrow->setFocus();
}

bool ErdConnection::isOwnerOf(ErdArrowItem* arrow)
{
    return arrow == this->arrow;
}

const ErdArrowItem* ErdConnection::getArrowItem() const
{
    return this->arrow;
}

bool ErdConnection::isCompoundConnection() const
{
    return !associatedConnections.isEmpty();
}

QList<ErdConnection*> ErdConnection::getAssociatedConnections() const
{
    return associatedConnections;
}

void ErdConnection::setAssociatedConnections(const QList<ErdConnection*>& connections)
{
    associatedConnections = connections;
}

void ErdConnection::setIndexInStartEntity(int idx)
{
    indexInStartEntity = idx;
    arrow->setArrowIndexInStartEntity(idx);
}

void ErdConnection::setIndexInEndEntity(int idx)
{
    indexInEndEntity = idx;
    arrow->setArrowIndexInEndEntity(idx);
}

ErdEntity* ErdConnection::getStartEntity() const
{
    return startEntity;
}

