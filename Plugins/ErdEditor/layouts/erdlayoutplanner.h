#ifndef ERDLAYOUTPLANNER_H
#define ERDLAYOUTPLANNER_H

#include <QList>
#include <QRectF>

class ErdLayout;
class ErdScene;
class ErdEntity;
class ErdLayoutPlanner
{
    public:
        enum class Algo
        {
            FDP,
            NEATO
        };

        ErdLayoutPlanner();

        void arrangeScene(ErdScene* scene, Algo algo);

    private:
        ErdLayout* produce(Algo algo);
        QList<ErdEntity*> entitiesOrderedForLayout(const QList<ErdEntity*>& entities);
};

#endif // ERDLAYOUTPLANNER_H
