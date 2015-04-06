#ifndef EXPIRINGCACHE_H
#define EXPIRINGCACHE_H

#include <QCache>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

template <class K, class V>
class ExpiringCache : public QCache<K, V>
{
    public:
        ExpiringCache(int maxCost = 100, int expireMs = 1000);
        ~ExpiringCache();

        bool insert(const K& key, V* object, int cost = 1);
        bool contains(const K& key) const;
        V*	object(const K& key, bool noExpireCheck = false) const;
        V*	operator[](const K& key) const;
        V*	take(const K& key);
        QList<K> keys() const;
        bool remove(const K& key);
        int	count() const;
        void clear();
        bool isEmpty() const;
        void setExpireTime(int ms);

    private:
        bool expired(const K& key) const;

        mutable QHash<K, qint64> expires;
        int expireMs;
};

template <class K, class V>
ExpiringCache<K, V>::ExpiringCache(int maxCost, int expireMs) :
    QCache<K, V>(maxCost), expireMs(expireMs)
{
}

template <class K, class V>
ExpiringCache<K, V>::~ExpiringCache()
{
}

template <class K, class V>
bool ExpiringCache<K, V>::insert(const K& key, V* object, int cost)
{
    QList<K> keysBefore = QCache<K, V>::keys();
    bool result = QCache<K, V>::insert(key, object, cost);
    if (!result)
        return false;

    QList<K> keysAfter = QCache<K, V>::keys();

    for (const K& keyBefore : keysBefore)
    {
        if (!keysAfter.contains(keyBefore))
            expires.remove(keyBefore);
    }

    expires[key] = QDateTime::currentMSecsSinceEpoch() + expireMs;
    return true;
}

template <class K, class V>
bool ExpiringCache<K, V>::contains(const K& key) const
{
    if (expired(key))
        return false;

    return QCache<K, V>::contains(key);
}

template <class K, class V>
V* ExpiringCache<K, V>::object(const K& key, bool noExpireCheck) const
{
    if (!noExpireCheck && expired(key))
        return nullptr;

    return QCache<K, V>::object(key);
}

template <class K, class V>
V* ExpiringCache<K, V>::operator[](const K& key) const
{
    if (expired(key))
        return nullptr;

    return QCache<K, V>::operator[](key);
}

template <class K, class V>
V* ExpiringCache<K, V>::take(const K& key)
{
    if (expired(key))
        return nullptr;

    expires.remove(key);
    return QCache<K, V>::take(key);
}

template <class K, class V>
QList<K> ExpiringCache<K, V>::keys() const
{
    QList<K> keyList;
    for (const K& key : QCache<K, V>::keys())
    {
        if (!expired(key))
            keyList << key;
    }
    return keyList;
}

template <class K, class V>
bool ExpiringCache<K, V>::remove(const K& key)
{
    expires.remove(key);
    return QCache<K, V>::remove(key);
}

template <class K, class V>
int ExpiringCache<K, V>::count() const
{
    return keys().count();
}

template <class K, class V>
void ExpiringCache<K, V>::clear()
{
    expires.clear();
    QCache<K, V>::clear();
}

template <class K, class V>
bool ExpiringCache<K, V>::isEmpty() const
{
    return keys().isEmpty();
}

template <class K, class V>
void ExpiringCache<K, V>::setExpireTime(int ms)
{
    expireMs = ms;
}

template <class K, class V>
bool ExpiringCache<K, V>::expired(const K& key) const
{
    if (expires.contains(key) && QDateTime::currentMSecsSinceEpoch() > expires[key])
    {
        expires.remove(key);
        return true;
    }
    return false;
}

#endif // EXPIRINGCACHE_H
