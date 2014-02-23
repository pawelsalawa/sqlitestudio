#ifndef SQLRESULTSROW_H
#define SQLRESULTSROW_H

#include "coreSQLiteStudio_global.h"
#include <QVariant>
#include <QList>
#include <QHash>
#include <QSharedPointer>

/** @file */

/**
 * @brief SQL query results row.
 *
 * Single row of data from SQL query results. It already has all columns stored in memory,
 * so it doesn't matter if you read only one column, or all columns available in the row.
 *
 * You will never encounter object of exactly this class, as it has protected constructor
 * and has no methods to populate internal data members. Instead of creating objects of this class,
 * other class inherits it and handles populating internal data members, then this class
 * is just an interface to read data from it.
 *
 * In other words, it's kind of an abstract class.
 */
class API_EXPORT SqlResultsRow
{
    public:
        /**
         * @brief Releases resources.
         */
        virtual ~SqlResultsRow();

        /**
         * @brief Gets value for given column.
         * @param key Column name.
         * @return Value from requested column. If column name is invalid, the invalid QVariant is returned.
         */
        QVariant& value(const QString& key);

        /**
         * @brief Gets value for given column.
         * @param idx 0-based index of column.
         * @return Value from requested column. If index was invalid, the invalid QVariant is returned.
         */
        QVariant value(int idx);

        /**
         * @brief Gets table of column->value entries.
         * @return Hash table with column names as keys and QVariants as their values.
         *
         * Note, that QHash doesn't guarantee order of entries. If you want to iterate through columns
         * in order they were returned from the database, use valueList(), or iterate through SqlResults::getColumnNames()
         * and use it to call value().
         */
        QHash<QString,QVariant>& valueMap();

        /**
         * @brief Gets list of values in this row.
         * @return Ordered list of values in the row.
         *
         * Note, that this method returns values in order they were returned from database.
         */
        QList<QVariant> valueList();

        /**
         * @brief Tests if the row contains given column name.
         * @param key Column name. Case sensitive.
         * @return true if column exists in the row, or false otherwise.
         */
        bool contains(const QString& key);

        /**
         * @brief Tests if the row has column indexed with given number.
         * @param idx 0-based index to test.
         * @return true if index is in range of existing columns, or false if it's greater than "number of columns - 1", or if it's less than 0.
         */
        bool contains(int idx);

    protected:
        SqlResultsRow();

        /**
         * @brief Columns and their values in the row.
         */
        QHash<QString,QVariant> valuesMap;
        /**
         * @brief Ordered list of values in the row.
         *
         * Technical note:
         * We keep list of values next to valuesMap, so we have it in the same order as column names when asked by valueList().
         * This looks like having redundant data storage, but Qt container classes (such as QVariant)
         * use smart pointers to keep their data internally, so here we actually keep only reference objects.
         */
        QList<QVariant> values;
};

/**
 * @brief Shared pointer to SQL query results row.
 */
typedef QSharedPointer<SqlResultsRow> SqlResultsRowPtr;

#endif // SQLRESULTSROW_H
