#ifndef BISTRHASH_H
#define BISTRHASH_H

#include "bihash.h"
#include <QHash>
#include <QString>

/**
 * @brief Bi-directional string-oriented hash.
 *
 * This hash is very similar to BiHash, except it always uses QString
 * for both left and right values. Given that, it also provides Qt::CaseSensitivity support
 * for any operations accepting values in parameters.
 *
 * Just like BiHash, the BiStrHash doesn't provide operator[]. For more details see BiHash.
 */
class BiStrHash
{
    public:
        /**
         * @brief Creates empty hash.
         */
        BiStrHash() {}

        /**
         * @brief Creates pre-initialized hash.
         * @param list C++11 style initializer list, like: <tt>{{"x"="y"}, {"a"="b"}}</tt>
         */
        BiStrHash(std::initializer_list<std::pair<QString, QString>> list)
        {
            hash = QHash<QString,QString>(list);
            initInvertedAndLower();
        }

        /**
         * @brief Creates BiStrHash basing on QHash.
         * @param other QHash to copy values from.
         *
         * Any conflicting values from the \p other hash will overwrite
         * current values in the hash.
         */
        BiStrHash(const QHash<QString, QString> & other)
        {
            unite(other);
        }

        /**
         * @brief Copy constructor.
         * @param other Other hash to copy.
         */
        BiStrHash(const BiStrHash& other) : hash(other.hash), inverted(other.inverted),
            lowerHash(other.lowerHash), lowerInverted(other.lowerInverted) {}

        /**
         * @brief Inserts entry into the hash.
         * @param left Left-side value to insert.
         * @param right Right-side value to insert.
         *
         * Inserting to the hash is done in case-insensitive manner, hence any conflicting
         * values matched with case insensitive method will be replaced with the new entry.
         */
        void insert(const QString& left, const QString& right)
        {
            if (lowerHash.contains(left.toLower()))
                removeLeft(left, Qt::CaseInsensitive);

            if (lowerInverted.contains(right.toLower()))
                removeRight(right, Qt::CaseInsensitive);

            inverted.insert(right, left);
            hash.insert(left, right);
            lowerHash.insert(left.toLower(), left);
            lowerInverted.insert(right.toLower(), right);
        }

        /**
         * @brief Tests if given value is in the left values of the hash.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return true if the key was matched in left side values, or false otherwise.
         */
        bool containsLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
                return hash.contains(left);
            else
                return lowerHash.contains(left.toLower());
        }

        /**
         * @brief Tests if given value is in the right values of the hash.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return true if the key was matched in right side values, or false otherwise.
         */
        bool containsRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
                return inverted.contains(right);
            else
                return lowerInverted.contains(right.toLower());
        }

        /**
         * @brief Removes entry matching given value in left-side values.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Number of entries removed.
         */
        int	removeLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                if (!hash.contains(left))
                    return 0;

                inverted.remove(hash.value(left));
                hash.remove(left);

                return 1;
            }
            else
            {
                QString lowerLeft = left.toLower();
                if (!lowerHash.contains(lowerLeft))
                    return 0;

                QString right = hash.value(lowerHash.value(lowerLeft));

                hash.remove(inverted.value(right));
                inverted.remove(right);
                lowerHash.remove(lowerLeft);
                lowerInverted.remove(right.toLower());

                return 1;
            }
        }

        /**
         * @brief Removes entry matching given value in right-side values.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Number of entries removed.
         */
        int	removeRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                if (!inverted.contains(right))
                    return 0;

                hash.remove(inverted.value(right));
                inverted.remove(right);

                return 1;
            }
            else
            {
                QString lowerRight = right.toLower();
                if (!lowerInverted.contains(lowerRight))
                    return 0;

                QString left = inverted.value(lowerInverted.value(lowerRight));

                inverted.remove(hash.value(left));
                hash.remove(left);
                lowerHash.remove(left.toLower());
                lowerInverted.remove(lowerRight);

                return 1;
            }
        }

        /**
         * @brief Removes entry from hash and returns it.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Right side value, or null string if the \p left was not matched.
         */
        QString takeLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                QString right = hash.take(left);
                inverted.remove(right);
                return right;
            }
            else
            {
                QString right = hash.take(lowerHash.take(left.toLower()));
                inverted.remove(lowerInverted.take(right.toLower()));
                return right;
            }
        }

        /**
         * @brief Removes entry from hash and returns it.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Left side value, or null string if the \p left was not matched.
         */
        QString takeRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive)
        {
            if (cs == Qt::CaseSensitive)
            {
                QString left = inverted.take(right);
                hash.remove(left);
                return left;
            }
            else
            {
                QString left = inverted.take(lowerInverted.take(right.toLower()));
                hash.remove(lowerHash.take(left.toLower()));
                return left;
            }
        }

        /**
         * @brief Copies all entries from the other hash to this hash.
         * @param other Other hash to copy values from.
         * @return Reference to this hash, after update.
         */
        BiStrHash& unite(const BiStrHash& other)
        {
            unite(other.hash);
            return *this;
        }

        /**
         * @overload
         */
        BiStrHash& unite(const QHash<QString,QString>& other)
        {
            QHashIterator<QString, QString> it(other);
            while (it.hasNext())
                insert(it.next().key(), it.value());

            return *this;
        }

        /**
         * @brief Finds right-side value by matching the left-side value.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Right-side value, or null string if left-side value was not matched.
         */
        QString valueByLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
        {
            if (cs == Qt::CaseSensitive)
                return hash.value(left);
            else
                return hash.value(lowerHash.value(left.toLower()));
        }

        /**
         * @brief Finds left-side value by matching the right-side value.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Left-side value, or null string if right-side value was not matched.
         */
        QString valueByRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
        {
            if (cs == Qt::CaseSensitive)
                return inverted.value(right);
            else
                return inverted.value(lowerInverted.value(right.toLower()));
        }

        /**
         * @brief Gives all left-side values.
         * @return List of values.
         */
        QStringList leftValues() const
        {
            return hash.keys();
        }

        /**
         * @brief Gives all right-side values.
         * @return List of values.
         */
        QStringList rightValues() const
        {
            return inverted.keys();
        }

        /**
         * @brief Provides java-style iterator for this hash.
         * @return Iterator object.
         */
        QHashIterator<QString,QString> iterator() const
        {
            return QHashIterator<QString,QString>(hash);
        }

        /**
         * @brief Removes all entries from the hash.
         */
        void clear()
        {
            hash.clear();
            inverted.clear();
            lowerHash.clear();
            lowerInverted.clear();
        }

        /**
         * @brief Counts all entries in the hash.
         * @return Number of entries in the hash.
         */
        int count() const
        {
            return hash.count();
        }

        /**
         * @brief Tests whether the hash is empty or not.
         * @return true if the hash is empty, false otherwise.
         */
        bool isEmpty() const
        {
            return hash.isEmpty();
        }

    private:
        /**
         * @brief Fills inverted and lower internal hashes basing on the main hash class member.
         */
        void initInvertedAndLower()
        {
            QHashIterator<QString,QString> it(hash);
            while (it.hasNext())
            {
                it.next();
                inverted[it.value()] = it.key();
                lowerHash[it.key().toLower()] = it.key();
                lowerInverted[it.value().toLower()] = it.value();
            }
        }

        /**
         * @brief Standard mapping - left to right.
         */
        QHash<QString,QString> hash;

        /**
         * @brief Right to left mapping.
         */
        QHash<QString,QString> inverted;

        /**
         * @brief Lower left to true left key mapping.
         */
        QHash<QString,QString> lowerHash;

        /**
         * @brief Lower right to true right key mapping.
         */
        QHash<QString,QString> lowerInverted;
};

#endif // BISTRHASH_H
