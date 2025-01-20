#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include <QStringList>

class ErdChange
{
    public:
        enum class Category
        {
            LAYOUT,
            ENTITY
        };

        ErdChange() = delete;

        virtual QStringList toDdl() = 0;

        Category getCategory() const;

    protected:
        ErdChange(Category category);

        Category category;
};

#endif // ERDCHANGE_H
