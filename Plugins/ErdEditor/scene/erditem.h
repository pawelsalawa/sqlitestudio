#ifndef ERDITEM_H
#define ERDITEM_H

class ErdItem
{
    public:
        virtual ~ErdItem();

        virtual bool isClickable() = 0;

        bool isBeingDeleted() const;
        void markAsBeingDeleted();

    private:
        bool beingDeleted = false;
};

#endif // ERDITEM_H
