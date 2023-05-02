#ifndef ERDGRAPHVIZLAYOUTPLANNER_H
#define ERDGRAPHVIZLAYOUTPLANNER_H

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
};

#endif // ERDGRAPHVIZLAYOUTPLANNER_H
