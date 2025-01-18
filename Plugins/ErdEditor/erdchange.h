#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include <QString>

class ErdChange
{
    public:
        enum class Category
        {
            LAYOUT,
            DDL
        };

        ErdChange() = delete;

        virtual QString toDdl() const = 0;

        Category getCategory() const;

    protected:
        ErdChange(Category category);

        Category category;
};

#endif // ERDCHANGE_H
