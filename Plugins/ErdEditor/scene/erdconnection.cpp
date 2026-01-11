#include "scene/erdconnection.h"
#include "scene/erdlinearrowitem.h"
#include "scene/erdcurvyarrowitem.h"
#include "scene/erdentity.h"
#include "scene/erdscene.h"
#include "erdeditorplugin.h"
#include "changes/erdchangeentity.h"
#include "db/db.h"
#include "db/chainexecutor.h"
#include "services/notifymanager.h"
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

    scene->removeItem(arrow);
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

    // Flush pre-cancel state
    preCancelEndEntity = nullptr;
    preCancelEndEntityRow = -1;
}

void ErdConnection::cancelFinalState(const QPointF& endPos)
{
    endEntity->removeConnection(this);
    endEntity->updateConnectionIndexes();
    preCancelEndEntity = endEntity;
    preCancelEndEntityRow = endEntityRow;
    endEntity = nullptr;
    endEntityRow = -1;
    updatePosition(endPos);
}

void ErdConnection::restoreFinalState()
{
    if (!preCancelEndEntity)
        return;

    endEntity = preCancelEndEntity;
    endEntityRow = preCancelEndEntityRow;
    endEntity->addConnection(this);
    endEntity->updateConnectionIndexes();
    preCancelEndEntity = nullptr;
    preCancelEndEntityRow = -1;
    refreshPosition();
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

    if (!removeCancelledFk(createTable) || !addFk(createTable))
    {
        notifyError(QObject::tr("Could not commit changes for finalized ERD connection."));
        scene->connectionFinalizationFailed();
        return;
    }

    createTable->rebuildTokens(); // needed for TableModifier when executing changes

    QString desc = preCancelEndEntity != nullptr ?
                QObject::tr("Update relationship from \"%1\"-\"%2\" to \"%1\"-\"%3\".")
                    .arg(createTable->table, preCancelEndEntity->getTableName(), endEntity->getTableName()) :
                QObject::tr("Create relationship between \"%1\" and \"%2\".")
                    .arg(createTable->table, endEntity->getTableName());

    ErdChange* change = new ErdChangeEntity(scene->getDb(), originalCreateTable, createTable, desc);

    ChainExecutor* ddlExecutor = new ChainExecutor();
    ddlExecutor->setTransaction(false);
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

bool ErdConnection::removeCancelledFk(SqliteCreateTablePtr& createTable)
{
    if (!preCancelEndEntity)
        return true;

    QString referencedTable = preCancelEndEntity->getTableName();
    QString srcColumn = getStartEntityColumn()->name;
    QString trgColumn = getPreCancelEndEntityColumn()->name;
    auto constrList = createTable->getColumnForeignKeysByTable(referencedTable, srcColumn, trgColumn);
    if (!constrList.isEmpty())
    {
        auto constr = constrList.first();
        createTable->removeColumnConstraint(constr);
        delete constr;
        return true;
    }

    auto tableConstrList = createTable->getForeignKeysByTable(referencedTable, {QPair<QString, QString>(srcColumn, trgColumn)});
    if (!tableConstrList.isEmpty())
    {
        auto constr = tableConstrList.first();
        createTable->constraints.removeOne(constr);
        delete constr;
        return true;
    }

    qCritical() << "Could not find FK constraint to remove while trying to commit ERD connection cancellation.";
    return false;
}

bool ErdConnection::addFk(SqliteCreateTablePtr& createTable)
{
    SqliteCreateTable::Column* startEntityColumn = getStartEntityColumn();
    if (!startEntityColumn)
    {
        qCritical() << "startEntityColumn is null while trying to commit ERD connection.";
        return false;
    }

    SqliteCreateTable::Column* endEntityColumn = getEndEntityColumn();
    if (!endEntityColumn)
    {
        qCritical() << "endEntityColumn is null while trying to commit ERD connection.";
        return false;
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

    return true;
}

bool ErdConnection::isTableLevelFk() const
{
    return tableLevelFk;
}

void ErdConnection::setTableLevelFk(bool value)
{
    tableLevelFk = value;
}

void ErdConnection::endEntityAboutToBeDeleted()
{
    endEntity = nullptr;
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

SqliteCreateTable::Column* ErdConnection::getPreCancelEndEntityColumn() const
{
    if (!preCancelEndEntity || preCancelEndEntityRow < 0)
        return nullptr;

    return dynamic_cast<SqliteCreateTable::Column*>(preCancelEndEntity->getStatementAtRowIndex(preCancelEndEntityRow));
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

void ErdConnection::deselect()
{
    arrow->setSelected(false);
}

void ErdConnection::markAsBeingDeleted()
{
    arrow->markAsBeingDeleted();
}

bool ErdConnection::isBeingDeleted() const
{
    return arrow->isBeingDeleted();
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

bool ErdConnection::isEditing() const
{
    return preCancelEndEntity != nullptr;
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

