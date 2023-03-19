#ifndef ERDSCENE_H
#define ERDSCENE_H

#include <QGraphicsScene>

class ErdScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        explicit ErdScene(QObject *parent = nullptr);
};

#endif // ERDSCENE_H
