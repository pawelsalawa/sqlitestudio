#include "erdscene.h"
#include "erdentity.h"
#include "schemaresolver.h"
#include "erdlinearrowitem.h"
#include "erdconnection.h"
#include "erdgraphvizlayoutplanner.h"
#include <QApplication>

ErdScene::ErdScene(QObject *parent)
    : QGraphicsScene{parent}
{

}

void ErdScene::parseSchema(Db* db)
{
    if (!db)
        return;

    SchemaResolver resolver(db);
    StrHash<SqliteCreateTablePtr> tables = resolver.getAllParsedTables();
    StrHash<ErdEntity*> entitiesByTable;
    for (SqliteCreateTablePtr& table : tables.values())
    {
        ErdEntity* entityItem = new ErdEntity(table);
        entityItem->setPos(lastCreatedX, -200);
        entityItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        addItem(entityItem);

        lastCreatedX += 150;

        entitiesByTable[table->table] = entityItem;
        entities << entityItem;
    }
    setupEntityConnections(entitiesByTable);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    for (ErdEntity*& entity : entities)
        entity->updateConnectionsGeometry();

    arrangeEntitiesFdp();
}

QList<ErdEntity*> ErdScene::getAllEntities() const
{
    return entities;
}

void ErdScene::setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable)
{
    for (ErdEntity*& srcEntity : entities)
    {
        setupEntityConnections(entitiesByTable, srcEntity);
        for (SqliteCreateTable::Column*& column : srcEntity->getTableModel()->columns)
            setupEntityConnections(entitiesByTable, srcEntity, column);
    }
}

void ErdScene::setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity)
{
    auto tableModel = srcEntity->getTableModel();
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

    ErdConnection* conn = new ErdConnection(srcEntity, srcRowIdx + 1, trgEntity, trgRowIdx + 1);
    conn->addToScene(this);
}

void ErdScene::refreshSceneRect()
{
    QRectF boundingRect = itemsBoundingRect();
    boundingRect.adjust(-sceneMargin, -sceneMargin, sceneMargin, sceneMargin);
    setSceneRect(boundingRect);
}

void ErdScene::newTable()
{
    // Create the first entity item
    SqliteCreateTable* tableModel = new SqliteCreateTable();
    tableModel->table = "test table " + QString::number(lastCreatedX);

    SqliteCreateTable::Column* col1 = new SqliteCreateTable::Column("test col", nullptr, {});
    col1->setParent(tableModel);
    tableModel->columns << col1;

    SqliteCreateTable::Column* col2 = new SqliteCreateTable::Column("another col", nullptr, {});
    col2->setParent(tableModel);
    tableModel->columns << col2;

    ErdEntity* entityItem = new ErdEntity(tableModel);
    entityItem->setPos(lastCreatedX, -200);
    entityItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    addItem(entityItem);

    lastCreatedX += 150;

    entities << entityItem;
}

void ErdScene::arrangeEntities(int algo)
{
    ErdGraphvizLayoutPlanner planner;
    planner.arrangeScene(this, static_cast<ErdGraphvizLayoutPlanner::Algo>(algo));
    update();
    refreshSceneRect();
}

void ErdScene::arrangeEntitiesFdp()
{
    arrangeEntities(ErdGraphvizLayoutPlanner::FDP);
}

void ErdScene::arrangeEntitiesNeato()
{
    arrangeEntities(ErdGraphvizLayoutPlanner::NEATO);
}
