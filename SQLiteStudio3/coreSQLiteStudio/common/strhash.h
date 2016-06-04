#ifndef STRHASH_H
#define STRHASH_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QDebug>

template <class T>
class StrHash
{
    public:
        StrHash() {}
        StrHash(std::initializer_list<std::pair<QString, T>> list) : hash(QHash<QString, T>(list))
        {
            initLower();
        }

        StrHash(const QHash<QString, T>& other) : hash(QHash<QString, T>(other))
        {
            initLower();
        }

        void insert(const QString& key, const T& value)
        {
            if (lowerCaseHash.contains(key.toLower()))
                remove(key, Qt::CaseInsensitive);

            hash.insert(key, value);
            lowerCaseHash.insert(key.toLower(), key);
        }

        bool contains(const QString& key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
        {
            if (cs == Qt::CaseSensitive)
                return hash.contains(key);

            return lowerCaseHash.contains(key.toLower());
        }

        int	remove(const QString& key, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                int res = hash.remove(key);
                if (res > 0)
                    lowerCaseHash.remove(key.toLower());

                return res;
            }

            // Case insensitive
            QString lowerKey = key.toLower();
            if (lowerCaseHash.contains(lowerKey))
            {
                int res = hash.remove(lowerCaseHash.value(lowerKey));
                lowerCaseHash.remove(lowerKey);
                return res;
            }

            return 0;
        }

        T take(const QString& key, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                lowerCaseHash.remove(key.toLower());
                return hash.take(key);
            }

            // Case insensitive
            QString lowerKey = key.toLower();
            if (lowerCaseHash.contains(lowerKey))
            {
                QString theKey = lowerCaseHash.value(lowerKey);
                lowerCaseHash.remove(lowerKey);
                return hash.take(theKey);
            }

            return QString();
        }

        StrHash<T>& unite(const StrHash<T>& other)
        {
            unite(other.hash);
            return *this;
        }

        StrHash<T>& unite(const QHash<QString, T>& other)
        {
            QHashIterator<QString, T> it(other);
            while (it.hasNext())
            {
                it.next();
                insert(it.key(), it.value());
            }

            return *this;
        }

        T value(const QString& key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
        {
            if (cs == Qt::CaseSensitive)
                return hash.value(key);

            return hash.value(lowerCaseHash.value(key.toLower()));
        }

        QList<T> values() const
        {
            return hash.values();
        }

        QStringList keys() const
        {
            return hash.keys();
        }

        QHashIterator<QString, T> iterator() const
        {
            return QHashIterator<QString, T>(hash);
        }

        void clear()
        {
            hash.clear();
            lowerCaseHash.clear();
        }

        int count() const
        {
            return hash.count();
        }

        int count(const QString& key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
        {
            if (cs == Qt::CaseSensitive)
                return hash.count(key);

            return lowerCaseHash.count(key.toLower());
        }

        bool isEmpty() const
        {
            return hash.isEmpty();
        }

        QHash<QString, T> toQHash() const
        {
            return hash;
        }

        T& operator[](const QString& key)
        {
            if (lowerCaseHash.contains(key.toLower()) && !hash.contains(key))
            {
                T value = hash[lowerCaseHash[key.toLower()]];
                remove(key, Qt::CaseInsensitive);
                hash[key] = value;
            }

            lowerCaseHash[key.toLower()] = key;
            return hash[key];
        }

        const T operator[](const QString& key) const
        {
            return hash[lowerCaseHash[key.toLower()]];
        }

    private:
        void initLower()
        {
            QHashIterator<QString, T> it(hash);
            while (it.hasNext())
            {
                it.next();
                lowerCaseHash[it.key().toLower()] = it.key();
            }
        }

        QHash<QString, QString> lowerCaseHash;
        QHash<QString, T> hash;
};

#endif // STRHASH_H
