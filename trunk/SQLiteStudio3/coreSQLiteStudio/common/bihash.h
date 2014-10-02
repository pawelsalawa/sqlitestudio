#ifndef BIHASH_H
#define BIHASH_H

#include <QHash>

/**
 * @brief Bi-directional QHash
 *
 * Bi-directional hash treats both inserted values as keys to each other.
 * Bi-directional hash built on the <tt>left</tt> and <tt>right</tt> values concept.
 *
 * It's not multi-value hash, so when you try to insert existing value to any
 * of sides (left or right), it will replace the whole conflicting entry.
 *
 * It doesn't provide operator[], because returning reference to an object,
 * which then can be changed outside would desynchronize internals of this hash
 * (internal inverted map could not be synchronized properly according to changes
 * to the external reference).
 */
template <class L, class R>
class BiHash
{
    public:
        /**
         * @brief Creates empty hash.
         */
        BiHash() {}

        /**
         * @brief Creates hash initialized with given values.
         * @param list C++11 style initializer list, like: <tt>{{x=y}, {a=b}}</tt>
         */
        BiHash(std::initializer_list<std::pair<L, R>> list)
        {
            hash = QHash<L,R>(list);
            initInverted();
        }

        /**
         * @brief Creates bi-hash basing on QHash.
         * @param other QHash to copy data from.
         *
         * If multiple keys in the QHash refer to the same value,
         * this is not defined which one of those values will remain,
         * but there will definitely be copied only one of them.
         */
        BiHash(const QHash<L, R> & other)
        {
            unite(other);
        }

        /**
         * @brief Creates copy of BiHash.
         * @param other BiHash to copy.
         */
        BiHash(const BiHash<L, R> & other) : hash(other.hash), inverted(other.inverted) {}

        /**
         * @brief Inserts new values to the hash.
         * @param left Left value to be inserted.
         * @param right Right value to be inserted.
         *
         * Note that if the \p left is already in left values, then the current entry
         * for the left will be replaced with this new one.
         * The same applies for the \p right.
         *
         * If it happens, that both \p left and \p right have already entries in the
         * hash, but those are 2 different entries, then the insert operation will
         * remove both conflicting records and insert the new one.
         */
        void insert(const L& left, const R& right)
        {
            if (hash.contains(left))
                removeLeft(left);

            if (inverted.contains(right))
                removeRight(right);

            inverted.insert(right, left);
            hash.insert(left, right);
        }

        /**
         * @brief Tests if left values contain given value.
         * @param left Value to test.
         * @return true if left values containe the \p left value.
         */
        bool containsLeft(const L& left) const
        {
            return hash.contains(left);
        }

        /**
         * @brief Tests if right values contain given value.
         * @param right Value to test.
         * @return true if right values containe the \p right value.
         */
        bool containsRight(const R& right) const
        {
            return inverted.contains(right);
        }

        /**
         * @brief Removes entry with left value equal to given value.
         * @param left Value to remove by.
         * @return Number of removed entries.
         */
        int	removeLeft(const L& left)
        {
            if (!hash.contains(left))
                return 0;

            inverted.remove(hash.value(left));
            hash.remove(left);

            return 1;
        }

        /**
         * @brief Removes entry with right value equal to given value.
         * @param right Value to remove by.
         * @return Number of removed entries.
         */
        int	removeRight(const R& right)
        {
            if (!inverted.contains(right))
                return 0;

            hash.remove(inverted.value(right));
            inverted.remove(right);

            return 1;
        }

        /**
         * @brief Pops entry with left value equal to given value.
         * @param left Value to pop by.
         * @return Right value assigned to given left value.
         */
        R takeLeft(const L& left)
        {
            R right = hash.take(left);
            inverted.remove(right);
            return right;
        }

        /**
         * @brief Pops entry with right value equal to given value.
         * @param right Value to pop by.
         * @return Left value assigned to given right value.
         */
        L takeRight(const R& right)
        {
            R left = inverted.take(right);
            hash.remove(left);
            return left;
        }

        /**
         * @brief Copies all entries from other BiHash to this hash.
         * @param other BiHash to copy from.
         * @return Reference to this hash after values are copied.
         *
         * Any entries from the \p other hash will overwrite current
         * entries in case of conflict (by either left or right value).
         */
        BiHash<L,R>& unite(const BiHash<L,R>& other)
        {
            unite(other.hash);
            return *this;
        }

        /**
         * @overload
         */
        BiHash<L,R>& unite(const QHash<L,R>& other)
        {
            QHashIterator<L, R> it(other);
            while (it.hasNext())
                insert(it.next().key(), it.value());

            return *this;
        }

        /**
         * @brief Finds right value associated with given left value.
         * @param left Left value to match.
         * @return Right value if found, or default constructed value of right type.
         */
        R valueByLeft(const L& left) const
        {
            return hash.value(left);
        }

        /**
         * @brief Finds left value associated with given right value.
         * @param right Right value to match.
         * @return Left value if found, or default constructed value of left type.
         */
        L valueByRight(const R& right) const
        {
            return inverted.value(right);
        }

        /**
         * @brief Provides all left values.
         * @return List of values from left side.
         */
        QList<L> leftValues() const
        {
            return hash.keys();
        }

        /**
         * @brief Provides all right values.
         * @return List of values from right side.
         */
        QList<R> rightValues() const
        {
            return inverted.keys();
        }

        /**
         * @brief Provides java-like iterator for the hash.
         * @return Iterator object for this hash.
         */
        QHashIterator<L,R> iterator() const
        {
            return QHashIterator<L,R>(hash);
        }

        /**
         * @brief Removes all entries from the hash.
         */
        void clear()
        {
            hash.clear();
            inverted.clear();
        }

        /**
         * @brief Counts all entries in the hash.
         * @return Number of entries.
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

        /**
         * @brief Provides QHash from this BiHash.
         * @return QHash with left values as keys.
         */
        const QHash<L,R>& toQHash() const
        {
            return hash;
        }

        /**
         * @brief Provides QHash with inverted values (right-to-left)
         * @return QHash with right values as keys.
         */
        const QHash<R,L>& toInvertedQHash() const
        {
            return inverted;
        }

    private:
        /**
         * @brief Fills inverted internal hash basing on values from hash class member.
         */
        void initInverted()
        {
            QHashIterator<L,R> it(hash);
            while (it.hasNext())
            {
                it.next();
                inverted[it.value()] = it.key();
            }
        }

        /**
         * @brief Hash containing left-to-right mapping.
         */
        QHash<L,R> hash;

        /**
         * @brief Hash containing right-to-left mapping.
         */
        QHash<R,L> inverted;
};

#endif // BIHASH_H
