#include "erdchange.h"

ErdChange::ErdChange(Category category) :
    category(category)
{

}

ErdChange::Category ErdChange::getCategory() const
{
    return category;
}
