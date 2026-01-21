#include "erdscene.h"
#include "erdentity.h"
#include "changes/erdchangedeleteentity.h"
#include "erdwindow.h"
#include "schemaresolver.h"
#include "erdlinearrowitem.h"
#include "erdconnection.h"
#include "erdgraphvizlayoutplanner.h"
#include "changes/erdchangemodifyentity.h"
#include "tablemodifier.h"
#include "changes/erdchangenewentity.h"
#include "erdview.h"
#include "changes/erdchangedeleteconnection.h"
#include "changes/erdchangecomposite.h"
#include "changes/erdchangecolorentity.h"
#include "db/chainexecutor.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include "uiutils.h"
#include <QMessageBox>
#include <QApplication>

ErdScene::ErdScene(ErdArrowItem::Type arrowType, QObject *parent)
    : QGraphicsScene{parent}, arrowType(arrowType)
{
    // Indexing causes significant amount of BSP tree crashes,
    // while it doesn't seem to have huge performance impact in our case.
    setItemIndexMethod(QGraphicsScene::NoIndex);

    ddlExecutor = new ChainExecutor(this);
    ddlExecutor->setAsync(false);
    ddlExecutor->setTransaction(false);
    ddlExecutor->setDisableForeignKeys(true);
    ddlExecutor->setDisableObjectDropsDetection(true);
    sceneChangeApi = new SceneChangeApiImpl(*this);
}

ErdScene::~ErdScene()
{
    safe_delete(schemaResolver);
    safe_delete(sceneChangeApi);
}

void ErdScene::setDb(Db *db)
{
    this->db = db;
    ddlExecutor->setDb(db);
    safe_delete(schemaResolver);
    schemaResolver = new SchemaResolver(db);
}

Db* ErdScene::getDb() const
{
    return db;
}

QSet<QString> ErdScene::parseSchema()
{
    if (!db)
        return QSet<QString>();

    QSet<QString> tableNames;
    StrHash<SqliteCreateTablePtr> tables = schemaResolver->getAllParsedTables();
    for (SqliteCreateTablePtr& table : tables.values())
    {
        if (isSystemTable(table->table))
            continue;

        tableNames << table->table.toLower();

        ErdEntity* entityItem = new ErdEntity(table);
        entityItem->setPos(getPosForNewEntity(entityItem));
        entityCreated(entityItem);
        addItem(entityItem);
    }
    setupEntityConnections();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    for (ErdEntity*& entity : entities)
        entity->updateConnectionsGeometry();

    return tableNames;
}

void ErdScene::handleChange(ErdChange* change)
{
    change->apply(*sceneChangeApi);
    changeApplied();
}

void ErdScene::applyItemFiltering(const QString& value)
{
    QList<ErdEntity*> entities = items() | NNMAP_CAST(ErdEntity*);
    for (ErdEntity*& entity : entities)
        entity->applyFilter(value);
}

void ErdScene::connectionFinalizationFailed()
{
    emit connectionEditAbortRequested();
}

void ErdScene::selectAll()
{
    QPainterPath path;
    path.addRect(sceneRect());

    setSelectionArea(path);
}

void ErdScene::clearScene()
{
    for (auto&& conn : getConnections())
        delete conn;

    for (ErdEntity*& entity : entities)
    {
        removeItem(entity);
        entityToBeDeleted(entity);
        delete entity;
    }

    clear(); // Just in case.
    refreshSceneRect();
}

void ErdScene::handleChangeRedo(ErdChange* change)
{
    change->applyRedo(*sceneChangeApi);
    changeApplied();
}

void ErdScene::handleChangeUndo(ErdChange* change)
{
    change->applyUndo(*sceneChangeApi);
    changeApplied();
}

void ErdScene::changeApplied()
{
    refreshScheduledConnections();
    emit sidePanelRefreshRequested();
}

void ErdScene::refreshSchemaForTableNames(const QStringList& tables)
{
    for (const QString& tableName : unique(tables))
    {
        ErdEntity* entity = entityMap[tableName];
        if (!entity)
        {
            SqliteCreateTablePtr createTable = SqliteCreateTablePtr::create();
            createTable->table = tableName;
            entity = new ErdEntity(createTable);
            entity->setPos(getPosForNewEntity(entity));
            entityCreated(entity);
            addItem(entity);
        }

        if (entity->isBeingDeleted())
            continue;

        refreshEntityFromTableName(entity, tableName);
    }
}

void ErdScene::setEntityPosition(ErdEntity* entity, const QPointF& pos)
{
    // Restore position, connections, etc
    entity->setPos(pos);
    entity->updateConnectionsGeometry();
    refreshSceneRect();
    emit showEntityToUser(entity);
}

void ErdScene::entityCreated(ErdEntity* entity)
{
    entities << entity;
    entityMap[entity->getTableName()] = entity;

    connect(entity, &ErdEntity::nameEdited, [this, entity](const QString& newName)
    {
        emit entityNameEditedInline(entity, newName);
    });
    connect(entity, &ErdEntity::fieldEdited, [this, entity](int colIdx, const QString& newName)
    {
        emit entityFieldEditedInline(entity, colIdx, newName);
    });
    connect(entity, &ErdEntity::fieldDeleted, [this, entity](int colIdx)
    {
        emit entityFieldDeletedInline(entity, colIdx);
    });
    connect(entity, &ErdEntity::requestVisibilityOf, this, &ErdScene::requestVisibilityOf);
    connect(entity, &ErdEntity::requestSceneGeomUpdate, this, &ErdScene::refreshSceneRect);
}

void ErdScene::entityToBeDeleted(ErdEntity* entity)
{
    entities.removeOne(entity);
    entityMap.remove(entity->getTableName());
}

void ErdScene::refreshScheduledConnections()
{
    for (ErdEntity*& entity : connectionRefreshScheduled)
    {
        if (entity->isBeingDeleted())
            continue;

        setupEntityConnections(entity);
    }
    connectionRefreshScheduled.clear();
}

void ErdScene::refreshEntityFromTableName(ErdEntity* entity, const QString& tableName)
{
    for (ErdConnection* conn : entity->getForeignConnections())
        connectionRefreshScheduled << conn->getStartEntity();

    entity->clearConnections();

    QString oldTableName = entity->getTableName();

    SqliteCreateTablePtr createTable = schemaResolver->getParsedTable(tableName);
    if (createTable.isNull())
    {
        removeEntityFromScene(entity);
        return;
    }

    entity->setTableModel(createTable);
    entity->setExistingTable(true);
    entity->modelUpdated();
    connectionRefreshScheduled << entity;

    entityMap.remove(oldTableName);
    entityMap.insert(tableName, entity);
}

QList<ErdEntity*> ErdScene::getAllEntities() const
{
    return entities;
}

QList<ErdEntity*> ErdScene::getSelectedEntities() const
{
    return entities | FILTER(entity, {return entity->isSelected();});
}

void ErdScene::setArrowType(ErdArrowItem::Type arrowType)
{
    if (this->arrowType == arrowType)
        return;

    this->arrowType = arrowType;
    for (ErdConnection* connection : getConnections())
        connection->setArrowType(arrowType);
}

ErdArrowItem::Type ErdScene::getArrowType() const
{
    return arrowType;
}

void ErdScene::applyConfig(const QHash<QString, QVariant>& erdConfig)
{
    QSet<ErdEntity*> noStoredPositionEntities;
    StrHash<QVariant> cfgEntities = erdConfig[CFG_KEY_ENTITIES].toHash();
    for (ErdEntity*& entity : entities)
    {
        QHash<QString, QVariant> singleEntityConfig = cfgEntities.value(entity->getTableName(), Qt::CaseInsensitive).toHash();
        if (singleEntityConfig.contains(CFG_KEY_POS))
        {
            QPointF pos(singleEntityConfig[CFG_KEY_POS].toPointF());
            entity->setPos(pos);
        }
        else
            noStoredPositionEntities << entity;

        if (singleEntityConfig.contains(CFG_KEY_COLOR))
        {
            QList<QVariant> colorList = singleEntityConfig[CFG_KEY_COLOR].toList();
            if (colorList.size() == 2)
            {
                QColor bg = colorList[0].value<QColor>();
                QColor fg = colorList[1].value<QColor>();
                entity->setCustomColor(bg, fg);
            }
        }
    }

    // Arrange entities without stored position
    if (!noStoredPositionEntities.isEmpty())
    {
        for (ErdEntity* entity : noStoredPositionEntities)
        {
            QPointF pos = getPosForNewEntitySpiral(entity, noStoredPositionEntities);
            entity->setPos(pos);
        }
    }

    QVariant cfgArrowType = erdConfig[ErdScene::CFG_KEY_ARROW_TYPE];
    if (!cfgArrowType.isNull())
        setArrowType((ErdArrowItem::Type)cfgArrowType.toInt());

    for (ErdConnection* conn : getConnections())
        conn->refreshPosition();

    update();
    refreshSceneRect();
}

QHash<QString, QVariant> ErdScene::getConfig()
{
    QHash<QString, QVariant> erdConfig;
    QHash<QString, QVariant> erdEntities;
    for (ErdEntity*& entity : entities)
    {
        if (!entity->isExistingTable())
            continue;

        auto colors = entity->getCustomColor();

        QHash<QString, QVariant> singleEntityConfig;
        singleEntityConfig[CFG_KEY_POS] = entity->pos();
        if (entity->usesCustomColor())
            singleEntityConfig[CFG_KEY_COLOR] = QList<QVariant>({colors.first, colors.second});

        erdEntities[entity->getTableName()] = singleEntityConfig;
    }
    erdConfig[CFG_KEY_ENTITIES] = erdEntities;
    erdConfig[CFG_KEY_ARROW_TYPE] = (int)arrowType;
    return erdConfig;
}

void ErdScene::setupEntityConnections()
{
    for (ErdEntity*& srcEntity : entities)
        setupEntityConnections(srcEntity);
}

void ErdScene::setupEntityConnections(ErdEntity* srcEntity)
{
    auto tableModel = srcEntity->getTableModel();

    // Table-level FKs
    auto constraints = tableModel->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY);
    QList<QList<ErdConnection*>> tableLevelFks;
    for (auto constr : constraints)
    {
        QList<ErdConnection*> singleFkConnections;
        int srcColNum = 0;
        for (SqliteIndexedColumn*& idxCol : constr->indexedColumns)
        {
            ErdConnection* conn = setupEntityConnection(
                srcEntity,
                idxCol->name,
                srcColNum++,
                constr->foreignKey
                );

            if (conn)
            {
                conn->setTableLevelFk(true);
                singleFkConnections << conn;
            }
        }
        if (!singleFkConnections.isEmpty())
            tableLevelFks << singleFkConnections;
    }

    // Make table-level FK connections aware of other connections that make the compound FK.
    for (QList<ErdConnection*>& singleFkConnections : tableLevelFks)
    {
        for (ErdConnection*& conn: singleFkConnections)
        {
            QList<ErdConnection*> associatedConnections = singleFkConnections;
            associatedConnections.removeOne(conn);
            conn->setAssociatedConnections(associatedConnections);
        }
    }

    // Column-level FKs
    for (SqliteCreateTable::Column*& column : srcEntity->getTableModel()->columns)
        setupEntityConnections(srcEntity, column);

    // Regenerate connection indexes for connection types that make use of it (i.e. square lines connection)
    srcEntity->updateConnectionIndexes();
}

void ErdScene::setupEntityConnections(ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn)
{
    auto constraints = srcColumn->getConstraints(SqliteCreateTable::Column::Constraint::FOREIGN_KEY);
    for (auto constr : constraints)
    {
        ErdConnection* conn = setupEntityConnection(
            srcEntity,
            srcColumn->name,
            0,
            constr->foreignKey
            );

        if (conn) // it may be null in case of invalid FK (inconsistent schema, referenced table does not exist)
            conn->setTableLevelFk(false);
    }
}

ErdConnection* ErdScene::setupEntityConnection(ErdEntity* srcEntity, const QString& srcColumn, int sourceReferenceIdx, SqliteForeignKey* fk)
{
    auto tableModel = srcEntity->getTableModel();
    QString fkTable = fk->foreignTable;
    if (!entityMap.contains(fkTable, Qt::CaseInsensitive))
    {
        qWarning() << "Foreign table" << fkTable << "is not known while parsing db schema for ERD.";
        qDebug() << "Known entities:" << entityMap.keys();
        return nullptr;
    }

    ErdEntity* trgEntity = entityMap.value(fkTable, Qt::CaseInsensitive);

    int srcRowIdx = tableModel->getColumnIndex(srcColumn);
    if (srcRowIdx == -1)
    {
        qWarning() << "Could not find column index of column" << srcColumn << "in source table"
                   << tableModel->table << "while parsing schema for ERD.";
    }

    QString fkCol;
    if (fk->indexedColumns.isEmpty())
    {
        // Implicit foreign column - same as source column
        fkCol = srcColumn;
    }
    else
    {
        if (sourceReferenceIdx >= fk->indexedColumns.size())
        {
            qWarning() << "More source columns than foreign columns in FK, while parsing schema for ERD. Table:" << tableModel->table;
            return nullptr;
        }

        fkCol = fk->indexedColumns[sourceReferenceIdx]->name;
    }
    int trgRowIdx = trgEntity->getTableModel()->getColumnIndex(fkCol);
    if (trgRowIdx == -1)
    {
        qWarning() << "Could not find column index of column" << fkCol << "in referenced table"
                   << fkTable << "while parsing schema for ERD.";
        return nullptr;
    }

    ErdConnection* conn = new ErdConnection(srcEntity, srcRowIdx + 1, trgEntity, trgRowIdx + 1, arrowType);
    conn->addToScene(this);
    return conn;
}

void ErdScene::refreshSceneRect()
{
    QRectF boundingRect = itemsBoundingRect();
    boundingRect.adjust(-sceneMargin, -sceneMargin, sceneMargin, sceneMargin);
    setSceneRect(boundingRect);
}

void ErdScene::notify(ErdChange *change)
{
    emit changeCreated(change);
}

void ErdScene::deleteItems(const QList<QGraphicsItem*>& items)
{
    if (items.isEmpty())
        return;

    // Mark entities as being deleted first to avoid persisting
    // "new entity yet to be commited" upon clearing side panel
    QList<ErdEntity*> entitiesToDelete = entities | FILTER(entity, {return items.contains(entity);});
    for (ErdEntity* entity : entitiesToDelete)
        entity->markAsBeingDeleted();

    emit sidePanelAbortRequested();

    for (auto&& item : items)
        item->setSelected(false);

    QList<ErdConnection*> connectionsToDelete = toList(getConnections()) |
        FILTER(conn,
        {
            if (!items.contains(conn->getArrowItem()))
                return false;

            if (entitiesToDelete.contains(conn->getStartEntity()))
                return false;

            if (entitiesToDelete.contains(conn->getEndEntity()))
                return false;

            return true;
        });

    // Sort by most owned connections per entity (children entities) first, so they get deleted first.
    // There will be less FK maintenance/renaming, so that resulting DDL is the simplest possible.
    sSort(entitiesToDelete, [](const ErdEntity* e1, const ErdEntity* e2)
    {
        return e1->getOwningConnections().size() > e2->getOwningConnections().size();
    });

    QList<ErdChange*> changes;
    for (ErdEntity*& e : entitiesToDelete)
    {
        ErdChange* change = deleteEntity(e);
        if (change)
            changes << change;
    }

    // Reduce compound connections to single connection per compound fk
    QList<ErdConnection*> handled;
    connectionsToDelete = connectionsToDelete | FILTER(conn,
        {
            if (handled.contains(conn))
                return false;

            handled << conn;
            handled += conn->getAssociatedConnections();
            return true;
        });

    for (ErdConnection*& c : connectionsToDelete)
    {
        ErdChange* change = deleteConnection(c);
        if (change)
            changes << change;
    }

    ErdChange* change = ErdChange::normalizeChanges(changes, tr("Delete multiple diagram elements.", "ERD editor"));
    if (change)
        emit changeCreated(change);
}

bool ErdScene::undoChange(ErdChange* change)
{
    if (!change)
        return false;

    QStringList undoDdl = change->getUndoDdl();
    if (!undoDdl.isEmpty())
    {
        ddlExecutor->setQueries(undoDdl);
        ddlExecutor->exec();
        if (!ddlExecutor->getSuccessfulExecution())
        {
            notifyError(tr("Failed to execute the undo DDL. Details: %1", "ERD editor")
                        .arg(ddlExecutor->getErrorsMessages().join("; ")));
            return false;
        }
    }
    handleChangeUndo(change);
    return true;
}

bool ErdScene::redoChange(ErdChange* change)
{
    if (!change)
        return false;

    QStringList ddl = change->toDdl();
    if (!ddl.isEmpty())
    {
        ddlExecutor->setQueries(ddl);
        ddlExecutor->setRollbackOnErrorTo(change->getTransactionId());
        ddlExecutor->exec();
        if (!ddlExecutor->getSuccessfulExecution())
        {
            notifyError(tr("Failed to execute the redo DDL. Details: %1", "ERD editor")
                        .arg(ddlExecutor->getErrorsMessages().join("; ")));
            return false;
        }
    }
    // Change handling should go through event queue
    QTimer::singleShot(0, this, [change, this]() {handleChangeRedo(change);});
    return true;
}

ErdChange* ErdScene::deleteEntity(ErdEntity*& entity)
{
    if (!entity->isExistingTable())
    {
        entity->markAsBeingDeleted();
        QTimer::singleShot(0, [this, entity]() {removeEntityFromScene(entity);});
        return nullptr;
    }

    QString changeDesc = ErdChangeDeleteEntity::defaultDescription(entity->getTableName());
    ErdChange* change = new ErdChangeDeleteEntity(db, entity->getTableName(), entity->pos(), entity->getCustomColor().first, changeDesc);
    ddlExecutor->setQueries(change->toDdl());
    ddlExecutor->setRollbackOnErrorTo(change->getTransactionId());
    ddlExecutor->exec();
    if (!ddlExecutor->getSuccessfulExecution())
    {
        delete change;
        notifyError(tr("Failed to execute DDL required for table deletion. Details: %1", "ERD editor")
                    .arg(ddlExecutor->getErrorsMessages().join("; ")));
        return nullptr;
    }

    entity->markAsBeingDeleted();
    return change;
}

ErdChange* ErdScene::deleteConnection(ErdConnection*& connection)
{
    QString changeDesc = tr("Delete foreign key between \"%1\" and \"%2\".")
            .arg(connection->getStartEntity()->getTableName(), connection->getEndEntity()->getTableName());
    ErdChange* change = new ErdChangeDeleteConnection(db, connection, changeDesc);
    ddlExecutor->setQueries(change->toDdl());
    ddlExecutor->setRollbackOnErrorTo(change->getTransactionId());
    ddlExecutor->exec();
    if (!ddlExecutor->getSuccessfulExecution())
    {
        delete change;
        notifyError(tr("Failed to execute DDL required for foreign key deletion. Details: %1")
                    .arg(ddlExecutor->getErrorsMessages().join("; ")));
        return nullptr;
    }

    connection->markAsBeingDeleted();
    return change;
}

void ErdScene::removeEntityFromScene(ErdEntity* entity)
{
    for (ErdConnection* conn : entity->getForeignConnections())
        conn->endEntityAboutToBeDeleted();

    for (ErdConnection* conn : entity->getOwningConnections())
        delete conn;

    removeItem(entity);
    entityToBeDeleted(entity);
    connectionRefreshScheduled.removeOne(entity);
    refreshSceneRect();

    delete entity;
}

void ErdScene::removeEntityFromSceneByName(const QString& tableName)
{
    QString lowerName = tableName.toLower();
    ErdEntity* entity = entities | FIND_FIRST(e, {return (e->getTableName().toLower() == lowerName);});
    if (!entity)
    {
        // Requested to remove table tableName from scene, but no such table was found.
        // It was most likely removed by the schema refresh for tables affected by the particular ErdChange.
        return;
    }

    removeEntityFromScene(entity);
}

void ErdScene::placeNewEntity(ErdEntity* entity, const QPointF& pos)
{
    entity->setPos(pos);
    addItem(entity);

    entityCreated(entity);

    refreshSceneRect();
}

ErdConnection* ErdScene::getConnectionForArrow(ErdArrowItem* arrow)
{
    if (!arrow)
        return nullptr;

    for (ErdConnection* conn : getConnections())
    {
        if (conn->isOwnerOf(arrow))
            return conn;
    }
    return nullptr;
}

void ErdScene::arrangeEntities(int algo)
{
    ErdGraphvizLayoutPlanner planner;
    planner.arrangeScene(this, static_cast<ErdGraphvizLayoutPlanner::Algo>(algo));
    refreshSceneRect();
    invalidate();
}

QPointF ErdScene::getPosForNewEntity(ErdEntity* entity, const QSet<ErdEntity*>& excludeFromCalculations) const
{
    UNUSED(entity);
    bool first = true;
    QRectF layoutRect;

    for (ErdEntity* e : entities)
    {
        if (!e || excludeFromCalculations.contains(e))
            continue;

        QRectF rect = e->sceneBoundingRect();
        if (first)
        {
            layoutRect = rect;
            first = false;
        }
        else
        {
            layoutRect = layoutRect.united(rect);
        }
    }

    if (first)
        return QPointF(0.0, 0.0);

    return QPointF(layoutRect.right() + 150.0, layoutRect.top());
}

QPointF ErdScene::getPosForNewEntitySpiral(ErdEntity* entity, const QSet<ErdEntity*>& excludeFromCalculations) const
{
    if (!entity)
        return QPointF(0, 0);

    // Calculate centroid of existing entities
    QPointF centroid(0, 0);
    int count = 0;

    for (ErdEntity* e : entities)
    {
        if (!e)
            continue;

        if (excludeFromCalculations.contains(e))
            continue;

        QRectF r = e->sceneBoundingRect();
        centroid += r.center();
        ++count;
    }

    if (count == 0)
        return QPointF(0, 0);

    centroid /= count;

    // Prepare entity size and local offset
    QRectF localRect = entity->boundingRect();
    QSizeF size = localRect.size();

    // Correction: boundingRect is local, while position refers to origin
    QPointF localOffset = localRect.topLeft();

    // Spiral parameters
    constexpr qreal step = 80.0;
    constexpr int maxRings = 50;

    // Spiral search
    for (int ring = 0; ring <= maxRings; ++ring)
    {
        for (int dx = -ring; dx <= ring; ++dx)
        {
            for (int dy = -ring; dy <= ring; ++dy)
            {
                // Only consider positions on the current ring
                if (qAbs(dx) != ring && qAbs(dy) != ring)
                    continue;

                QPointF candidateCenter = centroid + QPointF(dx * step, dy * step);
                QPointF candidatePos = candidateCenter - QPointF(size.width() / 2, size.height() / 2) - localOffset;
                QRectF candidateRect(candidatePos, size);
                if (!collides(candidateRect, excludeFromCalculations))
                    return candidatePos;
            }
        }
    }

    // Fallback
    return getPosForNewEntity(entity, excludeFromCalculations);
}

bool ErdScene::collides(const QRectF& candidate, const QSet<ErdEntity*>& exclude) const
{
    for (ErdEntity* e : entities)
    {
        if (!e)
            continue;

        if (exclude.contains(e))
            continue;

        if (e->sceneBoundingRect().intersects(candidate))
            return true;
    }
    return false;
}

QSet<ErdConnection*> ErdScene::getConnections() const
{
    QSet<ErdConnection*> connections;
    for (ErdEntity* entity : entities)
    {
        auto list = entity->getConnections();
        connections += QSet<ErdConnection*>(list.begin(), list.end());
    }

    return connections;
}

bool ErdScene::confirmLayoutChange() const
{
    QMessageBox::StandardButton res = QMessageBox::question(
                qobject_cast<QWidget*>(parent()),
                tr("Arrange entities"),
                tr("Are you sure you want to automatically arrange the entities on the diagram? "
                   "This action will overwrite the current layout, and any manual adjustments will be lost.")
            );

    return res == QMessageBox::Yes;
}

QList<ErdEntity*> ErdScene::applyColorToSelectedEntities(const QColor& color)
{
    QColor textColor = color.isValid() ? findContrastingColor(color) : color;
    QList<ErdEntity*> entities = (selectedItems() | NNMAP_CAST(ErdEntity*));
    QList<ErdChange*> changes;
    for (ErdEntity* entity : entities)
    {
        QString tableName = entity->getTableName();
        QColor oldColor = entity->getCustomColor().first;
        QString colorName = color.name(QColor::HexRgb).toUpper();
        changes << new ErdChangeColorEntity(tableName, oldColor, color,
                    tr("Change color of table \"%1\" to %2.").arg(tableName, colorName));

        entity->setCustomColor(color, textColor);
    }

    ErdChange* change = ErdChange::normalizeChanges(changes, tr("Change color of multiple tables."));
    if (change)
        emit changeCreated(change);

    return entities;
}

QString ErdScene::getNewEntityName(const QString& prefix, int startIdx) const
{
    QStringList existingNames = entityMap.keys();
    QString baseName = prefix;
    QString name = baseName;
    for (int i = startIdx; existingNames.contains(name); i++)
        name = baseName + QString::number(i);

    return name;
}

void ErdScene::arrangeEntitiesFdp(bool skipConfirm)
{
    if (skipConfirm || confirmLayoutChange())
        arrangeEntities(ErdGraphvizLayoutPlanner::FDP);
}

void ErdScene::arrangeEntitiesNeato(bool skipConfirm)
{
    if (skipConfirm || confirmLayoutChange())
        arrangeEntities(ErdGraphvizLayoutPlanner::NEATO);
}

ErdScene::SceneChangeApiImpl::SceneChangeApiImpl(ErdScene& scene) :
    scene(scene)
{
}

void ErdScene::SceneChangeApiImpl::refreshEntity(const QString& entityName, const QString& actualTableName)
{
    ErdEntity* entity = scene.entityMap[entityName];
    if (!entity)
    {
        qCritical() << "SceneChangeApiImpl::refreshEntity: No entity found for name" << entityName;
        return;
    }
    scene.refreshEntityFromTableName(entity, actualTableName);
}

void ErdScene::SceneChangeApiImpl::refreshEntitiesByTableNames(const QStringList& tables)
{
    scene.refreshSchemaForTableNames(tables);
}

void ErdScene::SceneChangeApiImpl::removeEntityFromScene(const QString& entityName)
{
    scene.removeEntityFromSceneByName(entityName);
}

void ErdScene::SceneChangeApiImpl::setEntityPosition(const QString& entityName, const QPointF& pos)
{
    ErdEntity* entity = scene.entityMap[entityName];
    if (!entity)
    {
        qCritical() << "SceneChangeApiImpl::setEntityPosition: No entity found for name" << entityName;
        return;
    }
    scene.setEntityPosition(entity, pos);
}

void ErdScene::SceneChangeApiImpl::setEntityColor(const QString& entityName, const QColor& color)
{
    ErdEntity* entity = scene.entityMap[entityName];
    if (!entity)
    {
        qCritical() << "SceneChangeApiImpl::setEntityColor: No entity found for name" << entityName;
        return;
    }
    entity->setCustomColor(color);
}

QPointF ErdScene::SceneChangeApiImpl::getEntityPosition(const QString& entityName)
{
    ErdEntity* entity = scene.entityMap[entityName];
    if (!entity)
    {
        qCritical() << "SceneChangeApiImpl::setEntityPosition: No entity found for name" << entityName;
        return QPointF();
    }
    return entity->pos();
}

QColor ErdScene::SceneChangeApiImpl::getEntityColor(const QString& entityName)
{
    ErdEntity* entity = scene.entityMap[entityName];
    if (!entity)
    {
        qCritical() << "SceneChangeApiImpl::getEntityColor: No entity found for name" << entityName;
        return QColor();
    }
    return entity->getCustomColor().first;
}

SchemaResolver& ErdScene::SceneChangeApiImpl::schemaResolver()
{
    return *scene.schemaResolver;
}

void ErdScene::SceneChangeApiImpl::updateScene()
{
    QTimer::singleShot(0, [this]() {scene.update();});
}
