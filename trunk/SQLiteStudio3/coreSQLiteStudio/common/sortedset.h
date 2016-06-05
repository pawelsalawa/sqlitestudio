#ifndef SORTEDSET_H
#define SORTEDSET_H

#include "common/sortedhash.h"

template <class T>
class SortedSet : private SortedHash<T, bool>
{
    public:
        bool contains(const T &value) const
        {
            return QHash<T, bool>::containsKey(value);
        }

        int	count() const
        {
            return size();
        }

        bool isEmpty() const
        {
            return SortedHash<T, bool>::isEmpty();
        }

        bool remove(const T& value)
        {
            return SortedHash<T, bool>::remove(value);
        }

        int	size() const
        {
            return QHash<T, bool>::size();
        }

        void swap(SortedSet<T>& other)
        {
            return SortedHash<T, bool>::swap(other);
        }

        SortedSet<T>& operator+=(const T& value)
        {
            SortedHash<T, bool>::insert(value, true);
            return *this;
        }

        SortedSet<T>& operator-=(const T& value)
        {
            SortedHash<T, bool>::remove(value);
            return *this;
        }

        QSet<T>& operator<<(const T &value)
        {
            SortedHash<T, bool>::insert(value, true);
            return *this;
        }

        QList<T> toList()
        {
            return SortedHash<T, bool>::keys();
        }
};

#endif // SORTEDSET_H
