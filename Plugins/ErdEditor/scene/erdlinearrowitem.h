#ifndef ERDLINEARROWITEM_H
#define ERDLINEARROWITEM_H

#include "erdarrowitem.h"

class ErdLineArrowItem : public ErdArrowItem
{
    public:
        ErdLineArrowItem();

        void setPoints(const QLineF& line, Side startEntitySide, Side endEntitySide) override;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                   QWidget *widget = nullptr) override;
};



#endif // ERDLINEARROWITEM_H
