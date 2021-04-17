#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include "coreSQLiteStudio_global.h"
#include <QList>
#include <QSet>
#include <QHash>

template <class T>
inline QSet<T> toSet(const QList<T>& list)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return QSet<T>(list.begin(), list.end());
#else
    return list.toSet();
#endif
}

template <class K, class V>
inline void unite(QHash<K, V>& h1, const QHash<K, V>& h2)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    h1.insert(h2);
#else
    h1.unite(h2);
#endif
}

template <class T>
void sSort(T& collection)
{
    std::sort(collection.begin(), collection.end());
}

template <class T, class C>
void sSort(T& collection, C cmp)
{
    std::sort(collection.begin(), collection.end(), cmp);
}

API_EXPORT void strSort(QStringList& collection, Qt::CaseSensitivity cs);

#endif // COMPATIBILITY_H
