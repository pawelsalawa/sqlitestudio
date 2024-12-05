#include "querygenerator.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "db/db.h"
#include "common/unused.h"

QueryGenerator::QueryGenerator()
{

}

QString QueryGenerator::generateSelectFromTable(Db* db, const QString& table, const StrHash<QVariantList> values)
{
    return generateSelectFromTable(db, QString(), table, values);
}

QString QueryGenerator::generateSelectFromTable(Db* db, const QString& database, const QString& table, const StrHash<QVariantList> values)
{
    SchemaResolver resolver(db);
    QStringList columns = resolver.getTableColumns(database, table);
    return generateSelectFromTableOrView(db, database, table, columns, values);
}

QString QueryGenerator::generateInsertToTable(Db* db, const QString& table, const StrHash<QVariantList> values)
{
    return generateInsertToTable(db, QString(), table, values);
}

QString QueryGenerator::generateInsertToTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values)
{
    static_qstring(tpl, "INSERT INTO %1 (%2) VALUES %3");
    static_qstring(rowTpl, "(%1)");

    QString target = toFullObjectName(database, table);

    // Get all table's columns
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // If no values were given, then column names will serve as values for insertion
    if (values.isEmpty())
    {
        QStringList valueList = wrapStrings(tableCols);
        QStringList wrappedCols = wrapObjNamesIfNeeded(tableCols);
        return tpl.arg(target, wrappedCols.join(", "), rowTpl.arg(valueList.join(", ")));
    }

    // Values were given. Sort given columns in order they are defined in table
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Group values into rows
    QStringList valueSets = toValueSets(valueCols, values);
    QString valueStr = rowTpl.arg(valueSets.join("), ("));

    // Wrap given column names
    QStringList wrappedCols = wrapObjNamesIfNeeded(valueCols);

    return tpl.arg(target, wrappedCols.join(", "), valueStr);
}

QString QueryGenerator::generateUpdateOfTable(Db* db, const QString& table, const StrHash<QVariantList> values)
{
    return generateUpdateOfTable(db, QString(), table, values);
}

QString QueryGenerator::generateUpdateOfTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values)
{
    static_qstring(tpl, "UPDATE %1 SET %2%3");
    static_qstring(tplWithWhere, "UPDATE %1 SET %2 WHERE %3");
    static_qstring(updateColTpl, "%1 = %2");

    QString target = toFullObjectName(database, table);

    // Get all columns of the table
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // Create list of "column = 'column'"
    QStringList commonUpdateCols;
    for (const QString& col : tableCols)
        commonUpdateCols << updateColTpl.arg(wrapObjIfNeeded(col), wrapString(col));

    // Put it to comma spearated string
    QString commonColumnStr = commonUpdateCols.join(", ");

    // If no values were given, we simply use column names as values everywhere
    if (values.isEmpty())
    {
        QString conditionStr = commonUpdateCols.join(" AND ");
        return tplWithWhere.arg(target, commonColumnStr, conditionStr);
    }

    // If values were given, then they will be used in WHERE clause
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values);

    return tpl.arg(target, commonColumnStr, conditionStr);
}

QString QueryGenerator::generateDeleteFromTable(Db* db, const QString& table, const StrHash<QVariantList> values)
{
    return generateDeleteFromTable(db, QString(), table, values);
}

QString QueryGenerator::generateDeleteFromTable(Db* db, const QString& database, const QString& table, StrHash<QVariantList> values)
{
    static_qstring(tpl, "DELETE FROM %1%2");
    static_qstring(tplWithWhere, "DELETE FROM %1 WHERE %2");
    static_qstring(conditionColTpl, "%1 = %2");

    QString target = toFullObjectName(database, table);

    // Get all columns of the table
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // If no values were given, we simply use column names as values everywhere
    if (values.isEmpty())
    {
        // Create list of "column = 'column'"
        QStringList conditionCols;
        for (const QString& col : tableCols)
            conditionCols << conditionColTpl.arg(wrapObjIfNeeded(col), wrapString(col));

        // Put it to comma spearated string
        QString conditionStr = conditionCols.join(" AND ");

        return tplWithWhere.arg(target, conditionStr);
    }

    // If values were given, then they will be used in WHERE clause
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values);

    return tpl.arg(target, conditionStr);
}

QString QueryGenerator::generateSelectFromView(Db* db, const QString& view, const StrHash<QVariantList> values)
{
    return generateSelectFromView(db, QString(), view, values);
}

QString QueryGenerator::generateSelectFromView(Db* db, const QString& database, const QString& view, const StrHash<QVariantList> values)
{
    SchemaResolver resolver(db);
    QStringList columns = resolver.getViewColumns(database, view);
    return generateSelectFromTableOrView(db, database, view, columns, values);
}

QString QueryGenerator::generateSelectFromSelect(Db* db, const QString& initialSelect, const StrHash<QVariantList> values, const BiStrHash& dbNameToAttach)
{
    static_qstring(tpl, "SELECT %1 FROM (%2)%3");

    // Resolve all columns of the select
    QList<SelectResolver::Column> columns = SelectResolver::sqliteResolveColumns(db, initialSelect, dbNameToAttach);

    // Generate result columns
    QStringList resCols;
    for (const SelectResolver::Column& col : columns)
        resCols << toResultColumnString(col);

    // Generate conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values);

    return tpl.arg(resCols.join(", "), initialSelect, conditionStr);
}

QString QueryGenerator::generateSelectFromTableOrView(Db* db, const QString& database, const QString& tableOrView, const QStringList& columns, const StrHash<QVariantList> values)
{
    UNUSED(db);
    static_qstring(tpl, "SELECT %1 FROM %2%3");

    QStringList wrappedCols = wrapObjNamesIfNeeded(columns);
    QString target = toFullObjectName(database, tableOrView);
    QString conditionStr = valuesToConditionStr(values);

    return tpl.arg(wrappedCols.join(", "), target, conditionStr);
}

QString QueryGenerator::generateSelectFunction(const QString& function, const QStringList& columns, const QHash<QString, QVariantList> values)
{
    // To make SQLite evaluate every function call just once, we're placing the
    // function calls in the VALUES clause (making them operate on literal arguments), e.g.:
    // WITH data ([upper(a)], [upper(b)]) AS (
    //     VALUES (upper('apple'), upper('banana'))
    // ) SELECT * from data;
    static_qstring(tpl, "WITH data (%1) AS (VALUES %2) SELECT * FROM data");
    static_qstring(functionCallTpl, "%1(\\1)");
    static_qstring(rowTpl, "(%1)");

    // Group values into rows
    QStringList valueSets = toValueSets(columns, values, function + "(%1)");
    QString valueStr = rowTpl.arg(valueSets.join("), ("));

    // Wrap given column names
    QStringList wrappedCols = wrapObjNamesIfNeeded(columns);

    // Create expressions
    QRegularExpression re("^(.*)$");
    QStringList expressions;
    for (QString col : wrappedCols)
       expressions << col.replace(re, functionCallTpl.arg(function));

    // Use expressions as column names
    QStringList resultWrappedCols = wrapObjNamesIfNeeded(expressions);

    return tpl.arg(resultWrappedCols.join(", "), valueStr);
}

QString QueryGenerator::getAlias(const QString& name, QSet<QString>& usedAliases)
{
    static_qstring(tpl, "%2%1");

    QString letter;
    if (name.length() == 0)
        letter = "t";
    else
        letter = name[0];

    QString alias = letter + "1";
    int i = 2;
    while (usedAliases.contains(alias))
        alias = tpl.arg(i++).arg(letter);

    usedAliases << alias;
    return alias;
}

QStringList QueryGenerator::valuesToConditionList(const StrHash<QVariantList>& values)
{
    static_qstring(conditionTpl0, "%1 IS NULL");
    static_qstring(conditionTpl1, "%1 = %2");
    static_qstring(conditionTpl2, "%1 IN (%2)");

    QStringList conditions;
    QStringList conditionValues;
    for (const QString& col : values.keys())
    {
        conditionValues = valueListToSqlList(values[col]);
        conditionValues.removeDuplicates();
        if (conditionValues.size() == 1)
        {
            if (conditionValues.first() == "NULL")
                conditions << conditionTpl0.arg(wrapObjIfNeeded(col));
            else
                conditions << conditionTpl1.arg(wrapObjIfNeeded(col), conditionValues.first());
        }
        else
            conditions << conditionTpl2.arg(wrapObjIfNeeded(col), conditionValues.join(", "));
    }
    return conditions;
}

QString QueryGenerator::valuesToConditionStr(const StrHash<QVariantList>& values)
{
    static_qstring(condTpl, " WHERE %1");

    QStringList conditions = valuesToConditionList(values);
    QString conditionStr = "";
    if (conditions.size() > 0)
        conditionStr = condTpl.arg(conditions.join(" AND "));

    return conditionStr;
}

QString QueryGenerator::toResultColumnString(const SelectResolver::Column& column)
{
    return wrapObjIfNeeded(column.displayName);
}

QString QueryGenerator::toFullObjectName(const QString& database, const QString& object)
{
    static_qstring(tpl, "%1%2");

    QString dbName = "";
    if (!database.isEmpty() && dbName.toLower() != "main")
        dbName = wrapObjIfNeeded(database);

    if (!dbName.isEmpty())
        dbName.append(".");

    return tpl.arg(dbName, wrapObjIfNeeded(object));
}

QStringList QueryGenerator::toValueSets(const QStringList& columns, const StrHash<QVariantList> values,
                                        const QString& format)
{
    QStringList rows;
    QVariantList rowValues;
    QStringList valueList;

    for (int total = values.values().first().size(), i = 0; i < total; i++)
    {
        rowValues.clear();
        for (const QString& col : columns)
            rowValues << values[col][i];

        valueList = valueListToSqlList(rowValues);
        QString row;
        for (QString value : valueList)
        {
            if (row.size() > 0)
                row.append(", ");
            row.append(format.isEmpty() ? value : format.arg(value));
        }
        rows << row;
    }

    return rows;
}
