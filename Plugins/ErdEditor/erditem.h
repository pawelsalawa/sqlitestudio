#ifndef ERDITEM_H
#define ERDITEM_H

class ErdItem
{
    public:
        virtual ~ErdItem();

        virtual bool isClickable() = 0;
};

#endif // ERDITEM_H
