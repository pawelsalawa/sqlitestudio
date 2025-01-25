#include "erdscene.h"
#include "erdentity.h"
#include "erdwindow.h"
#include "schemaresolver.h"
#include "erdlinearrowitem.h"
#include "erdconnection.h"
#include "erdgraphvizlayoutplanner.h"
#include "erdchangeentity.h"
#include "tablemodifier.h"
#include "erdchangenewentity.h"
#include "erdview.h"
#include <QApplication>
#include <QMessageBox>

ErdScene::ErdScene(ErdArrowItem::Type arrowType, QObject *parent)
    : QGraphicsScene{parent}, arrowType(arrowType)
{
}

QSet<QString> ErdScene::parseSchema(Db* db)
{
    if (!db)
        return QSet<QString>();

    QSet<QString> tableNames;
    SchemaResolver resolver(db);
    StrHash<SqliteCreateTablePtr> tables = resolver.getAllParsedTables();
    StrHash<ErdEntity*> entitiesByTable;
    for (SqliteCreateTablePtr& table : tables.values())
    {
        if (isSystemTable(table->table))
            continue;

        tableNames << table->table.toLower();

        ErdEntity* entityItem = new ErdEntity(table);
        entityItem->setPos(getPosForNewEntity());
        addItem(entityItem);

        entitiesByTable[table->table] = entityItem;
        entities << entityItem;
    }
    setupEntityConnections(entitiesByTable);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    for (ErdEntity*& entity : entities)
        entity->updateConnectionsGeometry();

    return tableNames;
}

void ErdScene::refreshSchema(Db *db, ErdChangeEntity* entityChange)
{
    StrHash<ErdEntity*> entitiesByTable = collectEntitiesByTable();

    typedef QPair<QString, QString> OldNewName;
    QList<OldNewName> modifiedTables = {
        OldNewName(entityChange->getTableNameBefore(), entityChange->getTableNameAfter())
    };
    for (const QString& tableName : entityChange->getTableModifier()->getModifiedTables())
        modifiedTables << OldNewName(tableName, tableName);

    SchemaResolver resolver(db);
    for (const OldNewName& oldNewTableName : modifiedTables)
    {
        ErdEntity* entity = entitiesByTable[oldNewTableName.first];
        refreshEntityFromTableName(resolver, entitiesByTable, entity, oldNewTableName.second);
    }
}

void ErdScene::refreshSchema(Db* db, ErdChangeNewEntity* newEntityChange)
{
    StrHash<ErdEntity*> entitiesByTable = collectEntitiesByTable();

    SchemaResolver resolver(db);
    refreshEntityFromTableName(resolver, entitiesByTable, newEntityChange->getEntity(), newEntityChange->getTableName());
}

void ErdScene::refreshEntityFromTableName(SchemaResolver& resolver, StrHash<ErdEntity*>& entitiesByTable, ErdEntity* entity, const QString& tableName)
{
    entity->clearConnections();

    QString oldTableName = entity->getTableName();

    SqliteCreateTablePtr createTable = resolver.getParsedTable(tableName);
    entity->setTableModel(createTable);
    entity->setExistingTable(true);
    entity->modelUpdated();

    entitiesByTable.remove(oldTableName);
    entitiesByTable.insert(tableName, entity);

    setupEntityConnections(entitiesByTable, entity);
}

QList<ErdEntity*> ErdScene::getAllEntities() const
{
    return entities;
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
    StrHash<QVariant> cfgEntities = erdConfig[CFG_KEY_ENTITIES].toHash();
    for (ErdEntity*& entity : entities)
    {
        QHash<QString, QVariant> singleEntityConfig = cfgEntities.value(entity->getTableName(), Qt::CaseInsensitive).toHash();
        if (singleEntityConfig.contains(CFG_KEY_POS))
        {
            QPointF pos(singleEntityConfig[CFG_KEY_POS].toPointF());
            entity->setPos(pos);
        }
        // TODO entity color
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

        QHash<QString, QVariant> singleEntityConfig;
        singleEntityConfig[CFG_KEY_POS] = entity->pos();
        // TODO entity color
        erdEntities[entity->getTableName()] = singleEntityConfig;
    }
    erdConfig[CFG_KEY_ENTITIES] = erdEntities;
    erdConfig[CFG_KEY_ARROW_TYPE] = (int)arrowType;
    return erdConfig;
}

void ErdScene::setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable)
{
    for (ErdEntity*& srcEntity : entities)
        setupEntityConnections(entitiesByTable, srcEntity);
}

void ErdScene::setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity)
{
    auto tableModel = srcEntity->getTableModel();

    // Table-level FKs
    auto constraints = tableModel->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY);
    for (auto constr : constraints)
    {
        int srcColNum = 0;
        for (SqliteIndexedColumn*& idxCol : constr->indexedColumns)
        {
            setupEntityConnection(
                entitiesByTable,
                srcEntity,
                idxCol->name,
                srcColNum++,
                constr->foreignKey
                );
        }
    }

    // Column-level FKs
    for (SqliteCreateTable::Column*& column : srcEntity->getTableModel()->columns)
        setupEntityConnections(entitiesByTable, srcEntity, column);

    // Regenerate connection indexes for connection types that make use of it (i.e. square lines connection)
    srcEntity->updateConnectionIndexes();
}

void ErdScene::setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn)
{
    auto constraints = srcColumn->getConstraints(SqliteCreateTable::Column::Constraint::FOREIGN_KEY);
    for (auto constr : constraints)
    {
        setupEntityConnection(
            entitiesByTable,
            srcEntity,
            srcColumn->name,
            0,
            constr->foreignKey
            );
    }
}

void ErdScene::setupEntityConnection(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, const QString& srcColumn, int sourceReferenceIdx, SqliteForeignKey* fk)
{
    auto tableModel = srcEntity->getTableModel();
    QString fkTable = fk->foreignTable;
    if (!entitiesByTable.contains(fkTable, Qt::CaseInsensitive))
    {
        qWarning() << "Foreign table" << fkTable << "is not known while parsing db schema for ERD.";
        return;
    }

    ErdEntity* trgEntity = entitiesByTable.value(fkTable, Qt::CaseInsensitive);

    if (sourceReferenceIdx >= fk->indexedColumns.size())
    {
        qWarning() << "More source columns than foreign columns in FK, while parsing schema for ERD. Table:" << tableModel->table;
        return;
    }

    int srcRowIdx = tableModel->getColumnIndex(srcColumn);
    if (srcRowIdx == -1)
    {
        qWarning() << "Could not find column index of column" << srcColumn << "in source table"
                   << tableModel->table << "while parsing schema for ERD.";
    }

    QString fkCol = fk->indexedColumns[sourceReferenceIdx]->name;
    int trgRowIdx = trgEntity->getTableModel()->getColumnIndex(fkCol);
    if (trgRowIdx == -1)
    {
        qWarning() << "Could not find column index of column" << fkCol << "in referenced table"
                   << fkTable << "while parsing schema for ERD.";
    }

    ErdConnection* conn = new ErdConnection(srcEntity, srcRowIdx + 1, trgEntity, trgRowIdx + 1, arrowType);
    conn->addToScene(this);
}

void ErdScene::refreshSceneRect()
{
    QRectF boundingRect = itemsBoundingRect();
    boundingRect.adjust(-sceneMargin, -sceneMargin, sceneMargin, sceneMargin);
    setSceneRect(boundingRect);
}

void ErdScene::removeEntityFromScene(ErdEntity* entity)
{
    removeItem(entity);
    entities.removeOne(entity);

    // The line below is necessary to avoid app crash after deleting the entity object.
    // Apparently without it Qt does not update the items in the internal tree of nodes
    // and because of that the deferred repaint event causes crash.
    // At least that's what it looks like from debugging (crash happens in the Qt's internal BSP Tree).
    refreshSceneRect();

    delete entity;
}

void ErdScene::removeEntityFromSceneByName(const QString& tableName)
{
    QString lowerName = tableName.toLower();
    ErdEntity* entity = findFirst<ErdEntity>(entities, [&lowerName](ErdEntity* e)
    {
        return (e->getTableName().toLower() == lowerName);
    });

    if (!entity)
    {
        qWarning() << "Requested to remoe entity" << tableName << "from scene, but no such entity was found!";
        return;
    }
    removeEntityFromScene(entity);
}

void ErdScene::placeNewEntity(ErdEntity* entity)
{
    entity->setPos(getPosForNewEntity());
    addItem(entity);

    entities << entity;

    refreshSceneRect();
    emit showEntityToUser(entity);
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
    update();
    refreshSceneRect();
}

QPointF ErdScene::getPosForNewEntity() const
{
    QRectF sceneRect = itemsBoundingRect();
    qreal posX = sceneRect.right();
    qreal posY = sceneRect.top();
    for (ErdEntity* entity : entities)
    {
        QRectF rect = entity->boundingRect();
        QPointF pos = entity->pos();
        posX = qMax(posX, pos.x() + rect.width());
        posY = qMin(posY, pos.y());
    }
    posX += 150;
    return QPointF(posX, posY);
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

StrHash<ErdEntity*> ErdScene::collectEntitiesByTable() const
{
    StrHash<ErdEntity*> hash;
    for (ErdEntity* entity : entities)
        hash[entity->getTableName()] = entity;

    return hash;
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
