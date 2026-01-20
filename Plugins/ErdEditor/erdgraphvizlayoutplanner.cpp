#include "scene/erdscene.h"
#include "erdgraphvizlayoutplanner.h"
#include "scene/erdentity.h"
#include "scene/erdconnection.h"
#include "uidebug.h"
#include <cfloat>
#include <QGuiApplication>
#include <QScreen>
#include <graphviz/gvc.h>

ErdGraphvizLayoutPlanner::ErdGraphvizLayoutPlanner()
{
}

void ErdGraphvizLayoutPlanner::arrangeScene(ErdScene* scene, Algo algo)
{
    QList<ErdEntity*> unsortedEntities = scene->getAllEntities();
    QList<ErdEntity*> entities = entitiesOrderedForLayout(unsortedEntities);

    QSet<ErdConnection*> connections;
    for (ErdEntity* entity : entities)
    {
        auto entityConns = entity->getConnections();
        connections += QSet(entityConns.begin(), entityConns.end());
    }

    // Init diagram
    GVC_t* gvc = gvContext();
    Agraph_t* graph = AGOPEN("ERD", Agdirected, nullptr);
    double marginPx = 0.0;
    if (algo == NEATO)
    {
        AGSAFESET(graph, "start", "self", "");
        AGSAFESET(graph, "overlap", "prism", "");
        AGSAFESET(graph, "pack", "30", "");
        marginPx = 10.0;
    }
    else
    {
        AGSAFESET(graph, "start", "random", "");
        AGSAFESET(graph, "pack", "30", "");
        marginPx = 40.0;
    }

    // Add nodes to graph
    QHash<ErdEntity*, Agnode_t*> entityNodes;
    QHash<QString, ErdEntity*> entityByName;
    static const qreal dpi = 72;
    for (ErdEntity*& entity : entities)
    {
        Agnode_t* node = agnode(graph, const_cast<char*>(entity->getTableName().toStdString().c_str()), 1);
        QRectF rect = entity->boundingRect();
        double w = (rect.width()  + marginPx) / dpi;
        double h = (rect.height() + marginPx) / dpi;
        AGSAFESET(node, "fixedsize", "true", "");
        AGSAFESET(node, "shape", "rectangle", "");
        AGSAFESET(node, "width", QString::number(w).toLatin1().constData(), "");
        AGSAFESET(node, "height", QString::number(h).toLatin1().constData(), "");
        entityNodes[entity] = node;
        entityByName[entity->getTableName()] = entity;
    }

    // Add edges to graph
    for (ErdConnection* conn : std::as_const(connections))
    {
        Agnode_t* node1 = entityNodes[conn->getStartEntity()];
        Agnode_t* node2 = entityNodes[conn->getEndEntity()];
        agedge(graph, node1, node2, nullptr, 1);
    }

    // Apply neato layout algorithm
    gvLayout(gvc, graph, algo == NEATO ? "neato" : "fdp");

    // Get node positions and update entities
    for (Agnode_t* node = agfstnode(graph); node; node = agnxtnode(graph, node))
    {
        double x = ND_pos(node)[0] * dpi;
        double y = ND_pos(node)[1] * dpi;

        ErdEntity* entity = entityByName[agnameof(node)];
        QRectF entityRect = entity->boundingRect();
        entity->setPos(
                    x - entityRect.width() / 2,
                    -y - entityRect.height() / 2
                );
        entity->updateConnectionsGeometry();
    }

    // Cleanup
    agclose(graph);
    gvFreeContext(gvc);

    if (isDebugEnabled())
        for (auto* a : entities)
            for (auto* b : entities)
                if (a != b && a->sceneBoundingRect().intersects(b->sceneBoundingRect()))
                    qDebug() << "ErdGraphvizLayoutPlanner OVERLAP:" << a->getTableName() << b->getTableName();
}

QList<ErdEntity*> ErdGraphvizLayoutPlanner::entitiesOrderedForLayout(const QList<ErdEntity*>& entities) const
{
    QList<ErdEntity*> orderedResult;
    QSet<ErdEntity*> handledEntities;
    while (orderedResult.size() < entities.size())
    {
        // Find entity with most connections among unhandled entities
        ErdEntity* startEntity = nullptr;
        int maxConnections = -1;
        for (ErdEntity* entity : entities)
        {
            if (handledEntities.contains(entity))
                continue;

            int connCount = entity->getConnections().size();
            if (connCount > maxConnections)
            {
                maxConnections = connCount;
                startEntity = entity;
            }
        }

        if (!startEntity)
            break;

        QSet<ErdEntity*> toProcess;
        toProcess.insert(startEntity);
        while (!toProcess.isEmpty())
        {
            // Find entity with most connections recursively
            ErdEntity* current = nullptr;
            int maxRecurrentConnections = -1;
            for (ErdEntity* entity : toProcess)
            {
                int connCount = entity->getConnections().size();
                if (connCount > maxRecurrentConnections)
                {
                    maxRecurrentConnections = connCount;
                    current = entity;
                }
            }

            if (!current)
                break;

            toProcess.remove(current);
            handledEntities.insert(current);
            orderedResult << current;

            // Add connected entities to process list
            for (ErdConnection* conn : current->getConnections())
            {
                ErdEntity* otherEntity = conn->getStartEntity() == current ?
                            conn->getEndEntity() : conn->getStartEntity();
                if (!handledEntities.contains(otherEntity))
                    toProcess.insert(otherEntity);
            }
        }
    }

    return orderedResult;
}
