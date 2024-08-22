#ifndef QUERYGENERATOR_H
#define QUERYGENERATOR_H

#include "common/column.h"
#include "common/bistrhash.h"
#include "schemaresolver.h"
#include "common/strhash.h"
#include <QString>
#include <QVariant>

class Db;

class API_EXPORT QueryGenerator
{
    public:
        QueryGenerator();

        /**
         * @brief Generates select of all column from the \p table having given column values matched.
         * @overload
         */
        QString generateSelectFromTable(Db* db, const QString& table, const StrHash<QVariantList> values = StrHash<QVariantList>());

        /**
         * @brief Generates SELECT of all column from the \p table having given column values matched.
         * @param db Database where the \p table exists.
         * @param database Attach name of the database (such as "main" or "temp", or any other used for ATTACH).
         * @param table Table to generate select for.
         * @param values Values to comply with.
         * @return SELECT statement string.
         *
         * Generates SELECT for given table, listing all result columns explicitly and adding WHERE clause (if \p values is not empty) with columns included in \p values
         * having any of values specified for those columns in that parameter.
         *
         * If \p values is ommited, then no WHERE clause is added.
         */
        QString generateSelectFromTable(Db* db, const QString& database, const QString& table, const StrHash<QVariantList> values = StrHash<QVariantList>());

        QString generateInsertToTable(Db* db, const QString& table, const StrHash<QVariantList> values = StrHash<QVariantList>());
        QString generateInsertToTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values = StrHash<QVariantList>());

        QString generateUpdateOfTable(Db* db, const QString& table, const StrHash<QVariantList> values = StrHash<QVariantList>());
        QString generateUpdateOfTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values = StrHash<QVariantList>());

        QString generateDeleteFromTable(Db* db, const QString& table, const StrHash<QVariantList> values = StrHash<QVariantList>());
        QString generateDeleteFromTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values = StrHash<QVariantList>());

        QString generateSelectFromView(Db* db, const QString& view, const StrHash<QVariantList> values = StrHash<QVariantList>());
        QString generateSelectFromView(Db* db, const QString& database, const QString& view, const StrHash<QVariantList> values = StrHash<QVariantList>());

        /**
         * @brief Generates SELECT for all columns from the \p initialSelect having values matched.
         * @param db Database that will be used to resolve tables used in the \p initialSelect
         * @param initialSelect The SELECT statement that will be used as a base for generating new query.
         * @param values Map of column names and values for them, so the WHERE clause is generated for them.
         * @param dbNameToAttach If the \p initialSelect uses attached databases, they should be provided in this parameter, so their symbolic names can be resolved to real attach name.
         * @return Generated SELECT statement string.
         *
         * Generates SELECT using \p initialSelect as a base. Lists all columns from \p initialSelect result columns explicitly (no star operator).
         * If there are \p values given, then the WHERE clause is added for them to match columns.
         */
        QString generateSelectFromSelect(Db* db, const QString& initialSelect, const StrHash<QVariantList> values = StrHash<QVariantList>(), const BiStrHash& dbNameToAttach = BiStrHash());

        /**
         * @brief Generates SELECT of a function applied to values
         * @param function The function name to apply to every value.
         * @param columns Ordered column names.
         * @param values Map of column names and values for them, for generating a VALUES clause.
         * @return Generated SELECT statement string.
         */
        QString generateSelectFunction(const QString& function, const QStringList& columns, const QHash<QString, QVariantList> values);

    private:
        QString generateSelectFromTableOrView(Db* db, const QString& database, const QString& tableOrView, const QStringList& columns, const StrHash<QVariantList> values = StrHash<QVariantList>());
        QString getAlias(const QString& name, QSet<QString>& usedAliases);
        QStringList valuesToConditionList(const StrHash<QVariantList>& values);
        QString valuesToConditionStr(const StrHash<QVariantList>& values);
        QString toResultColumnString(const SelectResolver::Column& column);
        QString toFullObjectName(const QString& database, const QString& object);
        QStringList toValueSets(const QStringList& columns, const StrHash<QVariantList> values,
                                const QString& format = QString());
};

#endif // QUERYGENERATOR_H
