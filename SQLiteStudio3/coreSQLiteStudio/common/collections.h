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

template <typename Fn>
ListFilterOp(Fn) -> ListFilterOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
QList<T> operator|(const QList<T>& list, const ListFilterOp<Predicate>& op)
{
    QList<T> out;
    for (const T& value : list)
        if (op.fn(value))
            out << value;

    return out;
}

template <typename T, typename Predicate>
QSet<T> operator|(const QSet<T>& list, const ListFilterOp<Predicate>& op)
{
    QSet<T> out;
    for (const T& value : list)
        if (op.fn(value))
            out << value;

    return out;
}

/**
 * Filter QList/QSet by given predicate body.
 * Example:
 * QList<int> values = {...};
 * QList<int> filtered = values | FILTER(val, {return val > 5;});
 */
#define FILTER(param, body) \
    ListFilterOp{[&](auto&& param) body}

/**
 * Shortcut for: FILTER(item, {return item != nullptr;})
 */
#define FILTER_NON_NULL() \
    FILTER(filterNonNullItem, {return filterNonNullItem != nullptr;})

// ==============================================
// MAP: list → mapped list

template <typename Fn>
struct ListMapOp {
    Fn fn;
};

template <typename Fn>
ListMapOp(Fn) -> ListMapOp<Fn>; // Needed by older clang

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

/**
 * Map QList/QSet to QList/QSet with template type concluded from implementation body.
 * Example:
 * QList<int> values = {...};
 * QList<QString> mapped = values | MAP(val, {return QString::number(val);});
 */
#define MAP(param, body) \
    ListMapOp{[&](auto&& param) body}

/**
 * Map QList/QSet to QList/QSet with template type concluded from implementation body.
 * The lambda used here does not capture any variables.
 */
#define MAP_NO_CAP(param, body) \
    ListMapOp{[](auto&& param) body}

/**
 * Cast QList/QSet to QList/QSet with specific template type using dynamic_cast.
 * Example:
 * QList<Type*> values = {...};
 * QList<ExtededType*> mapped = values | MAP_CAST(ExtededType*);
 */
#define MAP_CAST(type) \
    MAP(mapCastItem, {return dynamic_cast<type>(mapCastItem);})

/**
 * Cast QList/QSet to QList/QSet with specific template type using qobject_cast.
 */
#define MAP_QCAST(type) \
    MAP(mapCastItem, {return qobject_cast<type>(mapCastItem);})

// ==============================================
// NNMAP: list → mapped list, excluding null results from the map
// Usuable with pointers.

template <typename Fn>
struct ListNnMapOp {
        Fn fn;
};

template <typename Fn>
ListNnMapOp(Fn) -> ListNnMapOp<Fn>; // Needed by older clang

template <typename T, typename Mapper>
auto operator|(const QList<T>& list, const ListNnMapOp<Mapper>& op)
{
    using R = decltype(op.fn(std::declval<const T&>()));
    QList<R> out;
    for (const T& value : list)
    {
        R r = op.fn(value);
        if (r)
            out << r;
    }

    return out;
}

template <typename T, typename Mapper>
auto operator|(const QSet<T>& list, const ListNnMapOp<Mapper>& op)
{
    using R = decltype(op.fn(std::declval<const T&>()));
    QSet<R> out;
    for (const T& value : list)
    {
        R r = op.fn(value);
        if (r)
            out << r;
    }

    return out;
}

/**
 * Map QList/QSet to QList/QSet with template type concluded from implementation body.
 * If casted result is null, exclude it from resultsing collection.
 */
#define NNMAP(param, body) \
    ListNnMapOp{[&](auto&& param) body}

/**
 * Map QList/QSet to QList/QSet with template type concluded from implementation body.
 * The lambda used here does not capture any variables.
 * If casted result is null, exclude it from resultsing collection.
 */
#define NNMAP_NO_CAP(param, body) \
    ListNnMapOp{[](auto&& param) body}

/**
 * Cast QList/QSet to QList/QSet with specific template type using dynamic_cast.
 * If casted result is null, exclude it from resultsing collection.
 */
#define NNMAP_CAST(type) \
    NNMAP(mapCastItem, {return dynamic_cast<type>(mapCastItem);})

/**
 * Cast QList/QSet to QList/QSet with specific template type using qobject_cast.
 * If casted result is null, exclude it from resultsing collection.
 */
#define NNMAP_QCAST(type) \
    NNMAP(mapCastItem, {return qobject_cast<type>(mapCastItem);})

// ==============================================
// FIND_FIRST: list → optional-like pointer (nullptr = not found)
template <typename Fn>
struct ListFindFirstOp {
    Fn fn;
};

template <typename Fn>
ListFindFirstOp(Fn) -> ListFindFirstOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
T operator|(const QList<T>& list, const ListFindFirstOp<Predicate>& op)
{
    for (const T& value : list)
        if (op.fn(value))
            return value;

    return nullptr;
}

/**
 * Finds first occurrence of element complying with given predicate body
 * and returns it, or returns nullptr otherwise.
 */
#define FIND_FIRST(param, body) \
    ListFindFirstOp{[&](auto&& param) body}

// ==============================================
// FIND_LAST: list → optional-like pointer (nullptr = not found)
template <typename Fn>
struct ListFindLastOp {
        Fn fn;
};

template <typename Fn>
ListFindLastOp(Fn) -> ListFindLastOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
T operator|(const QList<T>& list, const ListFindLastOp<Predicate>& op)
{
    for (int i = list.size() - 1; i >= 0; --i)
        if (op.fn(list[i]))
            return list[i];

    return nullptr;
}

/**
 * Finds last occurrence of element complying with given predicate body
 * and returns it, or returns nullptr otherwise.
 */
#define FIND_LAST(param, body) \
    ListFindLastOp{[&](auto&& param) body}

// ==============================================
// INDEX_OF: list → index first occurrence
template <typename Fn>
struct ListIndexOfOp {
    Fn fn;
};

template <typename Fn>
ListIndexOfOp(Fn) -> ListIndexOfOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
int operator|(const QList<T>& list, const ListIndexOfOp<Predicate>& op)
{
    for (int i = 0; i < list.size(); ++i)
        if (op.fn(list[i]))
            return i;

    return -1;
}

/**
 * Finds first occcurrence of element complying with given predicate body
 * and returns index of it, or returns -1 otherwise.
 */
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

/**
 * Finds first occcurrence of string complying with given predicate body
 * and returns index of it, or returns -1 otherwise.
 * Since it's string-oriented, it supports starting index and case sensitivity
 * as 2nd and 3rd parameters.
 */
#define INDEX_OF_STR(value, ...) \
    ListIndexOfStrOp{value, ##__VA_ARGS__}

// ==============================================
// INDEXOF LAST: last occurrence
template <typename Fn>
struct ListIndexOfLastOp {
    Fn fn;
};

template <typename Fn>
ListIndexOfLastOp(Fn) -> ListIndexOfLastOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
int operator|(const QList<T>& list, const ListIndexOfLastOp<Predicate>& op)
{
    for (int i = list.size() - 1; i >= 0; --i)
        if (op.fn(list[i]))
            return i;

    return -1;
}

/**
 * Finds last occcurrence of element complying with given predicate body
 * and returns index of it, or returns -1 otherwise.
 */
#define LAST_INDEX_OF(param, body) \
    ListIndexOfLastOp{[&](auto&& param) body}

// ==============================================
// CONTAINS
template <typename Fn>
struct ListContainsOp {
    Fn fn;
};

template <typename Fn>
ListContainsOp(Fn) -> ListContainsOp<Fn>; // Needed by older clang

template <typename T, typename Predicate>
bool operator|(const QList<T>& list, const ListContainsOp<Predicate>& op)
{
    for (const T& value : list)
        if (op.fn(value))
            return true;

    return false;
}

template <typename T, typename Predicate>
bool operator|(const QSet<T>& list, const ListContainsOp<Predicate>& op)
{
    for (const T& value : list)
        if (op.fn(value))
            return true;

    return false;
}

/**
 * Checks whether the list contains an element complying with given predicate body
 * and then returns true if it does so.
 */
#define CONTAINS(param, body) \
    ListContainsOp{[&](auto&& param) body}

// ==============================================
// TO_HASH

template <typename Fn>
struct ListToHashOp {
    Fn fn;
};

template <typename Fn>
ListToHashOp(Fn) -> ListToHashOp<Fn>; // Needed by older clang

template <typename FnKey, typename FnVal>
struct ListToHashOp2 {
    FnKey keyFn;
    FnVal valFn;
};

template <typename FnKey, typename FnVal>
ListToHashOp2(FnKey, FnVal) -> ListToHashOp2<FnKey, FnVal>; // Needed by older clang

template <typename V, typename Mapper>
auto operator|(const QList<V>& list, const ListToHashOp<Mapper>& op)
{
    using K = decltype(op.fn(std::declval<const V&>()));
    QHash<K, V> result;
    for (const V& el : list)
        result[op.fn(el)] = el;

    return result;
}

template <typename V, typename KeyFn, typename ValFn>
auto operator|(const QList<V>& list, const ListToHashOp2<KeyFn, ValFn>& op)
{
    using K = decltype(op.keyFn(std::declval<const V&>()));
    using R = decltype(op.valFn(std::declval<const V&>()));

    QHash<K, R> result;
    for (const V& el : list)
        result[op.keyFn(el)] = op.valFn(el);

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

template <typename V, typename KeyFn, typename ValFn>
auto operator|(const QSet<V>& list, const ListToHashOp2<KeyFn, ValFn>& op)
{
    using K = decltype(op.keyFn(std::declval<const V&>()));
    using R = decltype(op.valFn(std::declval<const V&>()));

    QHash<K, R> result;
    for (const V& el : list)
        result[op.keyFn(el)] = op.valFn(el);

    return result;
}

#define _TO_HASH_2(param, key_body) \
    ListToHashOp{[&](auto&& param) key_body}

#define _TO_HASH_3(param, key_body, value_body) \
    ListToHashOp2{ \
        [&](auto&& param) key_body, \
        [&](auto&& param) value_body \
    }

/**
 * Reduces given collection to a QHash, mapping element's to a key by the given body.
 * If there is a collision on keys, the new one will always replace previos one.
 * If you provide 3rd parameter, it will be a body for translating element to a hash value.
 *
 * Example:
 * QList<Type*> values = {...};
 * QHash<int, Type*> hash = values | TO_HASH(item, {return item->getIntKey();});
 * QHash<int, QString> hash = values | TO_HASH(item, {return item->getIntKey();}, {return item->getStringValue();});
 */
#define TO_HASH(...) \
    _GET_MACRO(__VA_ARGS__, _TO_HASH_3, _TO_HASH_2)(__VA_ARGS__)

// ==============================================
// GROUP_BY

template <typename Fn>
struct ListGroupByOp {
    Fn fn;
};

template <typename Fn>
ListGroupByOp(Fn) -> ListGroupByOp<Fn>; // Needed by older clang

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

/**
 * Reduces given collection to a QHash, mapping element's to a key by the given body.
 * Values are stored in sub-collection of same type as input collection.
 * If there is a collision on keys, the resuling sub-collection for the same key will
 * contain multiple elements.
 * Example:
 * QList<Type*> values = {...};
 * QHash<int, QList<Type*>> hash = values | TO_HASH(item, {return item->getIntKey();});
 */
#define GROUP_BY(param, key_mapper_body) \
    ListGroupByOp{[&](auto&& param) key_mapper_body}


#endif // COLLECTIONS_H
