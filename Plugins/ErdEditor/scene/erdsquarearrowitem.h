#ifndef ERDSQUAREARROWITEM_H
#define ERDSQUAREARROWITEM_H

#include "erdarrowitem.h"

class ErdSquareArrowItem : public ErdArrowItem
{
    public:
        ErdSquareArrowItem();

        void setPoints(const QLineF& line, Side startSide, Side endSide) override;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                   QWidget *widget = nullptr) override;

    private:
        static constexpr int INITIAL_GAP = 20;
        static constexpr int PER_INDEX_SHIFT = 10;
};

#endif // ERDSQUAREARROWITEM_H
