#ifndef ERDCURVYARROWITEM_H
#define ERDCURVYARROWITEM_H

#include "erdarrowitem.h"

class ErdCurvyArrowItem : public ErdArrowItem
{
    public:
        ErdCurvyArrowItem();

        void setPoints(const QLineF& line, Side startEntitySide, Side endEntitySide) override;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                   QWidget *widget = nullptr) override;
};

#endif // ERDCURVYARROWITEM_H
