#ifndef ERDGRAPHVIZLAYOUTPLANNER_H
#define ERDGRAPHVIZLAYOUTPLANNER_H

#include <QSet>

#define AGSAFESET(graph, p1, p2, p3) agsafeset(graph, const_cast<char*>(p1), const_cast<char*>(p2), const_cast<char*>(p3))
#define AGOPEN(p1, p2, p3) agopen(const_cast<char*>(p1), p2, p3)

class ErdEntity;
class ErdScene;

class ErdGraphvizLayoutPlanner
{
    public:
        enum Algo
        {
            FDP,
            NEATO
        };

        ErdGraphvizLayoutPlanner();

        void arrangeScene(ErdScene* scene, Algo algo);

    private:
        QList<ErdEntity*> entitiesOrderedForLayout(const QList<ErdEntity*>& entities) const;
};

#endif // ERDGRAPHVIZLAYOUTPLANNER_H
