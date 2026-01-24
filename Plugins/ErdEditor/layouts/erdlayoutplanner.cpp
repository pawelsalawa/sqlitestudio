#include "erdlayoutplanner.h"
#include <QtMath>
#include <QHash>
#include <QSet>
#include "scene/erdentity.h"
#include "scene/erdconnection.h"
#include "scene/erdscene.h"
#include "layouts/erdlayoutneato.h"
#include "layouts/erdlayoutfdp.h"

ErdLayoutPlanner::ErdLayoutPlanner()
{

}

void ErdLayoutPlanner::arrangeScene(ErdScene* scene, Algo algo)
{
    QList<ErdEntity*> unsortedEntities = scene->getAllEntities();
    QList<ErdEntity*> entities = entitiesOrderedForLayout(unsortedEntities);

    QScopedPointer<ErdLayout> layout(produce(algo));
    layout->arrange(entities);

    QRectF rect = ErdLayout::computeBoundingBox(entities);
    scene->setSceneRect(rect.adjusted(-160, -160, 160, 160));
}

ErdLayout* ErdLayoutPlanner::produce(Algo algo)
{
    switch (algo)
    {
        case Algo::FDP:
            return new ErdLayoutFdp();
        case Algo::NEATO:
            return new ErdLayoutNeato();
    }
    return nullptr;
}

QList<ErdEntity*> ErdLayoutPlanner::entitiesOrderedForLayout(const QList<ErdEntity*>& entities)
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
