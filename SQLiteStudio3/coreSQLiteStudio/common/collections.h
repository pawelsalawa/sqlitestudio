#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "coreSQLiteStudio_global.h"
#include <QList>
#include <QStringList>
#include <QString>

// ==============================================
// FILTER: list → filtered list
template <typename Fn>
struct ListFilterOp {
    Fn fn;
};

template <typename T, typename Predicate>
QList<T> operator|(const QList<T>& list, const ListFilterOp<Predicate>& op)
{
    QList<T> out;
    for (const T& value : list)
        if (op.fn(value))
            out << value;

    return out;
}

#define FILTER(param, body) \
    ListFilterOp{[&](auto&& param) body}

// ==============================================
// MAP: list → mapped list

template <typename Fn>
struct ListMapOp {
    Fn fn;
};

template <typename T, typename Mapper>
auto operator|(const QList<T>& list, const ListMapOp<Mapper>& op)
{
    using R = decltype(op.fn(std::declval<const T&>()));
    QList<R> out;
    for (const T& value : list)
        out << op.fn(value);

    return out;
}

template <typename T, typename Mapper>
auto operator|(const QSet<T>& list, const ListMapOp<Mapper>& op)
{
    using R = decltype(op.fn(std::declval<const T&>()));
    QSet<R> out;
    for (const T& value : list)
        out << op.fn(value);

    return out;
}

#define MAP(param, body) \
    ListMapOp{[&](auto&& param) body}

#define MAP_NO_CAP(param, body) \
    ListMapOp{[](auto&& param) body}

// ==============================================
// FIND_FIRST: list → optional-like pointer (nullptr = not found)
template <typename Fn>
struct ListFindFirstOp {
    Fn fn;
};

template <typename T, typename Predicate>
T operator|(const QList<T>& list, const ListFindFirstOp<Predicate>& op)
{
    for (const T& value : list)
        if (op.fn(value))
            return value;

    return nullptr;
}

#define FIND_FIRST(param, body) \
    ListFindFirstOp{[&](auto&& param) body}

// ==============================================
// INDEX_OF: list → index first occurrence
template <typename Fn>
struct ListIndexOfOp {
    Fn fn;
};

template <typename T, typename Predicate>
int operator|(const QList<T>& list, const ListIndexOfOp<Predicate>& op)
{
    for (int i = 0; i < list.size(); ++i)
        if (op.fn(list[i]))
            return i;

    return -1;
}

#define INDEX_OF(param, body) \
    ListIndexOfOp{[&](auto&& param) body}

// ==============================================
// INDEX_OF_STR: string list → index first occurrence, string-oriented

struct API_EXPORT ListIndexOfStrOp {
    QString value;
    int from = 0;
    Qt::CaseSensitivity cs = Qt::CaseSensitive;
};

API_EXPORT int operator|(const QStringList& list, const ListIndexOfStrOp& op);

#define INDEX_OF_STR(value, ...) \
    ListIndexOfStrOp{value, ##__VA_ARGS__}

// ==============================================
// INDEXOF LAST: last occurrence
template <typename Fn>
struct ListIndexOfLastOp {
    Fn fn;
};

template <typename T, typename Predicate>
int operator|(const QList<T>& list, const ListIndexOfLastOp<Predicate>& op)
{
    for (int i = list.size() - 1; i >= 0; --i)
        if (op.fn(list[i]))
            return i;

    return -1;
}

#define LAST_INDEX_OF(param, body) \
    ListIndexOfLastOp{[&](auto&& param) body}

// ==============================================
// CONTAINS
template <typename Fn>
struct ListContainsOp {
    Fn fn;
};

template <typename T, typename Predicate>
bool operator|(const QList<T>& list, const ListContainsOp<Predicate>& op)
{
    for (const T& value : list)
        if (op.fn(value))
            return true;

    return false;
}

#define CONTAINS(param, body) \
    ListContainsOp{[&](auto&& param) body}

// ==============================================
// TO_HASH

template <typename Fn>
struct ListToHashOp {
    Fn fn;
};

template <typename V, typename Mapper>
auto operator|(const QList<V>& list, const ListToHashOp<Mapper>& op)
{
    using K = decltype(op.fn(std::declval<const V&>()));
    QHash<K, V> result;
    for (const V& el : list)
        result[op.fn(el)] = el;

    return result;
}

template <typename V, typename Mapper>
auto operator|(const QSet<V>& list, const ListToHashOp<Mapper>& op)
{
    using K = decltype(op.fn(std::declval<const V&>()));
    QHash<K, V> result;
    for (const V& el : list)
        result[op.fn(el)] = el;

    return result;
}

#define TO_HASH(param, key_mapper_body) \
    ListToHashOp{[&](auto&& param) key_mapper_body}

// ==============================================
// GROUP_BY

template <typename Fn>
struct ListGroupByOp {
    Fn fn;
};

template <typename V, typename Mapper>
auto operator|(const QList<V>& list, const ListGroupByOp<Mapper>& op)
{
    using K = decltype(op.fn(std::declval<const V&>()));
    QHash<K, QList<V>> result;
    for (const V& el : list)
        result[op.fn(el)] << el;

    return result;
}

template <typename V, typename Mapper>
auto operator|(const QSet<V>& list, const ListGroupByOp<Mapper>& op)
{
    using K = decltype(op.fn(std::declval<const V&>()));
    QHash<K, QSet<V>> result;
    for (const V& el : list)
        result[op.fn(el)] << el;

    return result;
}

#define GROUP_BY(param, key_mapper_body) \
    ListGroupByOp{[&](auto&& param) key_mapper_body}


#endif // COLLECTIONS_H
