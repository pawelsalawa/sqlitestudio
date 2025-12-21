#include "scene/erdscene.h"
#include "erdgraphvizlayoutplanner.h"
#include "scene/erdentity.h"
#include "scene/erdconnection.h"
#include <QGuiApplication>
#include <QScreen>
#include <graphviz/gvc.h>

ErdGraphvizLayoutPlanner::ErdGraphvizLayoutPlanner()
{
}

void ErdGraphvizLayoutPlanner::arrangeScene(ErdScene* scene, Algo algo)
{
    QList<ErdEntity*> entities = scene->getAllEntities();
    QSet<ErdConnection*> connections;
    for (ErdEntity* entity : entities)
    {
        auto entityConns = entity->getConnections();
        connections += QSet(entityConns.begin(), entityConns.end());
    }

    // Init diagram
    GVC_t* gvc = gvContext();
    Agraph_t* graph = AGOPEN("ERD", Agdirected, nullptr);
    AGSAFESET(graph, "sep", "0.5", "");
    if (algo == NEATO)
    {
        AGSAFESET(graph, "sep", "0.3", "");
        AGSAFESET(graph, "overlap", "scalexy", "");
    }

    // Add nodes to graph
    QHash<ErdEntity*, Agnode_t*> entityNodes;
    QHash<QString, ErdEntity*> entityByName;
    static const qreal dpi = 72;
    for (ErdEntity*& entity : entities)
    {
        Agnode_t* node = agnode(graph, const_cast<char*>(entity->getTableName().toStdString().c_str()), 1);
        QSize size = entity->rect().toRect().size();
        AGSAFESET(node, "fixedsize", "true", "");
        AGSAFESET(node, "shape", "rectangle", "");
        AGSAFESET(node, "width", QString::number(size.width() / dpi).toLatin1().constData(), "");
        AGSAFESET(node, "height", QString::number(size.height() / dpi).toLatin1().constData(), "");
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
        QRectF entityRect = entity->rect();
        entity->setPos(x - entityRect.width() / 2, y - entityRect.height() / 2);
        entity->updateConnectionsGeometry();
    }

    // Cleanup
    agclose(graph);
    gvFreeContext(gvc);
}
