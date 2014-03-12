#ifndef SORTEDHASH_H
#define SORTEDHASH_H

#include <QHash>

/**
 * @brief Partially implemented sorted hash.
 *
 * This is kind of a sorted QHash, except it doesn't act as sorted with iterators,
 * just with keys. It also doesn't work with multiple values for single key.
 *
 * The complete sorted hash might be implemented later on.
 */
template <class Key, class T>
class SortedHash : public QHash<Key, T>
{
    public:
        SortedHash(std::initializer_list<std::pair<Key, T>> list) : QHash<Key, T>(list)
        {
            sortedKeys = keys();
        }

        SortedHash(const QHash<Key, T>& other) : QHash<Key, T>(other)
        {
            sortedKeys = keys();
        }

        SortedHash(QHash<Key, T>&& other) : QHash<Key, T>(other)
        {
            sortedKeys = keys();
        }

        SortedHash() : QHash<Key, T>()
        {
        }

        typename QHash<Key, T>::iterator insert(const Key& key, const T& value)
        {
            if (!sortedKeys.contains(key))
                sortedKeys << key;

            return QHash<Key, T>::insert(key, value);
        }

        int remove(const Key& key)
        {
            sortedKeys.removeOne(key);
            return QHash<Key, T>::remove(key);
        }

        void swap(QHash<Key, T>& other)
        {
            QHash<Key, T>::swap(other);
            sortedKeys = keys();
        }

        void swap(SortedHash<Key, T>& other)
        {
            QHash<Key, T>::swap(other);
            sortedKeys = other.sortedKeys;
        }

        T take(const Key& key)
        {
            sortedKeys.removeOne(key);
            return QHash<Key, T>::take(key);
        }

        QList<Key> keys() const
        {
            return sortedKeys;
        }

        QList<Key> keys(const T& value) const
        {
            QList<Key> results;
            foreach (const Key& k, sortedKeys)
                if (value(k) == value)
                    results << k;

            return results;
        }

        SortedHash<Key, T>& unite(const QHash<Key, T>& other)
        {
            QHash<Key, T>::unite(other);
            sortedKeys += other.keys();
            return *this;
        }

        SortedHash<Key, T>& unite(const SortedHash<Key, T>& other)
        {
            QHash<Key, T>::unite(other);
            sortedKeys += other.sortedKeys;
            return *this;
        }

        QList<T> values() const
        {
            QList<T> results;
            foreach (const Key& k, sortedKeys)
                results << value(k);

            return results;
        }

        bool operator!=(const SortedHash<Key, T>& other) const
        {
            return !operator==(other);
        }

        SortedHash<Key, T>& operator=(const QHash<Key, T>& other)
        {
            QHash<Key, T>::operator=(other);
            sortedKeys = other.keys();
            return *this;
        }

        SortedHash<Key, T>& operator=(const SortedHash<Key, T>& other)
        {
            QHash<Key, T>::operator=(other);
            sortedKeys = other.sortedKeys;
            return *this;
        }

        bool operator==(const SortedHash<Key, T>& other) const
        {
            return QHash<Key, T>::operator==(other) && sortedKeys == other.sortedKeys;
        }

        T &	operator[](const Key& key)
        {
            if (!sortedKeys.contains(key))
                sortedKeys << key;

            return QHash<Key, T>::operator[](key);
        }

        const T	operator[](const Key& key) const
        {
            return QHash<Key, T>::operator[](key);
        }

        Key firstKey() const
        {
            if (sortedKeys.size() == 0)
                return Key();

            return sortedKeys.first();
        }

        Key lastKey() const
        {
            if (sortedKeys.size() == 0)
                return Key();

            return sortedKeys.last();
        }

    private:
        QList<Key> sortedKeys;
};

#endif // SORTEDHASH_H
