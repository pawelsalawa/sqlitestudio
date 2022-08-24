#ifndef BISTRHASH_H
#define BISTRHASH_H

#include "coreSQLiteStudio_global.h"
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
class API_EXPORT BiStrHash
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
        BiStrHash(std::initializer_list<std::pair<QString, QString>> list);

        /**
         * @brief Creates BiStrHash basing on QHash.
         * @param other QHash to copy values from.
         *
         * Any conflicting values from the \p other hash will overwrite
         * current values in the hash.
         */
        BiStrHash(const QHash<QString, QString> & other);

        /**
         * @brief Copy constructor.
         * @param other Other hash to copy.
         */
        BiStrHash(const BiStrHash& other);

        /**
         * @brief Inserts entry into the hash.
         * @param left Left-side value to insert.
         * @param right Right-side value to insert.
         *
         * Inserting to the hash is done in case-insensitive manner, hence any conflicting
         * values matched with case insensitive method will be replaced with the new entry.
         */
        void insert(const QString& left, const QString& right);

        /**
         * @brief Tests if given value is in the left values of the hash.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return true if the key was matched in left side values, or false otherwise.
         */
        bool containsLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Tests if given value is in the right values of the hash.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return true if the key was matched in right side values, or false otherwise.
         */
        bool containsRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Removes entry matching given value in left-side values.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Number of entries removed.
         */
        int	removeLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Removes entry matching given value in right-side values.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Number of entries removed.
         */
        int	removeRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Removes entry from hash and returns it.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Right side value, or null string if the \p left was not matched.
         */
        QString takeLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Removes entry from hash and returns it.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Left side value, or null string if the \p left was not matched.
         */
        QString takeRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive);

        /**
         * @brief Copies all entries from the other hash to this hash.
         * @param other Other hash to copy values from.
         * @return Reference to this hash, after update.
         */
        BiStrHash& unite(const BiStrHash& other);

        /**
         * @overload
         */
        BiStrHash& unite(const QHash<QString,QString>& other);

        /**
         * @brief Finds right-side value by matching the left-side value.
         * @param left Left-side value to match.
         * @param cs Case sensitivity flag.
         * @return Right-side value, or null string if left-side value was not matched.
         */
        QString valueByLeft(const QString& left, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Finds right-side value by matching the left-side value or returns defaultValue if not matched.
         * @param left Left-side value to match.
         * @param defaultValue Value to be returned if requested left key cannot be found.
         * @param cs Case sensitivity flag.
         * @return Right-side value, or provided default value if left-side value was not matched.
         */
        QString valueByLeft(const QString& left, const QString& defaultValue, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Finds left-side value by matching the right-side value.
         * @param right Right-side value to match.
         * @param cs Case sensitivity flag.
         * @return Left-side value, or null string if right-side value was not matched.
         */
        QString valueByRight(const QString& right, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Finds left-side value by matching the right-side value or returns defaultValue if not matched.
         * @param right Right-side value to match.
         * @param defaultValue Value to be returned if requested right key cannot be found.
         * @param cs Case sensitivity flag.
         * @return Left-side value, or provided default value if right-side value was not matched.
         */
        QString valueByRight(const QString& right, const QString& defaultValue, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

        /**
         * @brief Gives all left-side values.
         * @return List of values.
         */
        QStringList leftValues() const;

        /**
         * @brief Gives all right-side values.
         * @return List of values.
         */
        QStringList rightValues() const;

        /**
         * @brief Provides java-style iterator for this hash.
         * @return Iterator object.
         */
        QHashIterator<QString,QString> iterator() const;

        /**
         * @brief Removes all entries from the hash.
         */
        void clear();

        /**
         * @brief Counts all entries in the hash.
         * @return Number of entries in the hash.
         */
        int count() const;

        /**
         * @brief Tests whether the hash is empty or not.
         * @return true if the hash is empty, false otherwise.
         */
        bool isEmpty() const;

    private:
        /**
         * @brief Fills inverted and lower internal hashes basing on the main hash class member.
         */
        void initInvertedAndLower();

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
