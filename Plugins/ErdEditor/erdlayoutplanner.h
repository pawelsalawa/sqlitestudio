#ifndef ERDLAYOUTPLANNER_H
#define ERDLAYOUTPLANNER_H

#include <QList>

class ErdScene;

class ErdLayoutPlanner
{
    public:
        virtual ~ErdLayoutPlanner();

        virtual void arrangeScene(ErdScene* scene) = 0;
};

#endif // ERDLAYOUTPLANNER_H
