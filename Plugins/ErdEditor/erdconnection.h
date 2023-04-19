#ifndef ERDCONNECTION_H
#define ERDCONNECTION_H

#include <QObject>
#include <QPointF>

class ErdEntity;
class ErdArrowItem;

class ErdConnection
{
    public:
        ErdConnection(ErdEntity* startEntity, const QPointF& endPos);
        ErdConnection(ErdEntity* startEntity, int startRow, ErdEntity* endEntity, int endRow);
        virtual ~ErdConnection();

        ErdArrowItem* getArrow() const;
        void updatePosition(const QPointF& endPos);
        void finalizeConnection(ErdEntity* entity, const QPointF& endPos);
        bool isFinalized() const;
        void refreshPosition();

    private:
        QLineF calcPositions();

        static QPointF findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition);

        ErdEntity* startEntity = nullptr;
        ErdEntity* endEntity = nullptr;
        int startEntityRow = -1;
        int endEntityRow = -1;
        QPointF volatileEndPosition;
        ErdArrowItem* arrow = nullptr;
};

#endif // ERDCONNECTION_H
