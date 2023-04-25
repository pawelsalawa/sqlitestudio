#ifndef ERDGRAPHVIZLAYOUTPLANNER_H
#define ERDGRAPHVIZLAYOUTPLANNER_H

#include "erdlayoutplanner.h"

class ErdGraphvizLayoutPlanner : public ErdLayoutPlanner
{
    public:
        ErdGraphvizLayoutPlanner();

        void arrangeScene(ErdScene* scene);
};

#endif // ERDGRAPHVIZLAYOUTPLANNER_H
