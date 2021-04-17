#include "compatibility.h"

void strSort(QStringList& collection, Qt::CaseSensitivity cs)
{
    std::stable_sort(collection.begin(), collection.end(), [cs](const QString& v1, const QString& v2) -> bool
    {
        return v1.compare(v2, cs) < 0;
    });
}
