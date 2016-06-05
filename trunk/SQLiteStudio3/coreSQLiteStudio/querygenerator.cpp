#include "querygenerator.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "db/db.h"

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

    Dialect dialect = db->getDialect();
    QString target = toFullObjectName(database, table, dialect);

    // Get all table's columns
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // If no values were given, then column names will serve as values for insertion
    if (values.isEmpty())
    {
        QStringList valueList = wrapStrings(tableCols);
        QList<QString> wrappedCols = wrapObjNamesIfNeeded(tableCols, dialect);
        return tpl.arg(target, wrappedCols.join(", "), rowTpl.arg(valueList.join(", ")));
    }

    // Values were given. Sort given columns in order they are defined in table
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Group values into rows
    QStringList valueSets = toValueSets(valueCols, values, dialect);
    QString valueStr = rowTpl.arg(valueSets.join("), ("));

    // Wrap given column names
    QList<QString> wrappedCols = wrapObjNamesIfNeeded(valueCols, dialect);

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

    Dialect dialect = db->getDialect();
    QString target = toFullObjectName(database, table, dialect);

    // Get all columns of the table
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // Create list of "column = 'column'"
    QStringList commonUpdateCols;
    for (const QString& col : tableCols)
        commonUpdateCols << updateColTpl.arg(wrapObjIfNeeded(col, dialect), wrapString(col));

    // Put it to comma spearated string
    QString commonColumnStr = commonUpdateCols.join(", ");

    // If no values were given, we simply use column names as values everywhere
    if (values.isEmpty())
        return tplWithWhere.arg(target, commonColumnStr, commonColumnStr);

    // If values were given, then they will be used in WHERE clause
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values, dialect);

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

    Dialect dialect = db->getDialect();
    QString target = toFullObjectName(database, table, dialect);

    // Get all columns of the table
    SchemaResolver resolver(db);
    QStringList tableCols = resolver.getTableColumns(database, table);

    // If no values were given, we simply use column names as values everywhere
    if (values.isEmpty())
    {
        // Create list of "column = 'column'"
        QStringList conditionCols;
        for (const QString& col : tableCols)
            conditionCols << conditionColTpl.arg(wrapObjIfNeeded(col, dialect), wrapString(col));

        // Put it to comma spearated string
        QString conditionStr = conditionCols.join(", ");

        return tplWithWhere.arg(target, conditionStr);
    }

    // If values were given, then they will be used in WHERE clause
    QStringList valueCols = values.keys();
    sortWithReferenceList(valueCols, tableCols);

    // Conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values, dialect);

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

    Dialect dialect = db->getDialect();

    // Resolve all columns of the select
    SelectResolver resolver(db, initialSelect, dbNameToAttach);
    QList<SelectResolver::Column> columns = resolver.resolveColumnsFromFirstCore();

    // Generate result columns
    QStringList resCols;
    for (const SelectResolver::Column& col : columns)
        resCols << toResultColumnString(col, dialect);

    // Generate conditions for WHERE clause
    QString conditionStr = valuesToConditionStr(values, dialect);

    return tpl.arg(resCols.join(", "), initialSelect, conditionStr);
}

QString QueryGenerator::generateSelectFromTableOrView(Db* db, const QString& database, const QString& tableOrView, const QStringList& columns, const StrHash<QVariantList> values)
{
    static_qstring(tpl, "SELECT %1 FROM %2%3");

    Dialect dialect = db->getDialect();

    QString target = toFullObjectName(database, tableOrView, dialect);
    QString conditionStr = valuesToConditionStr(values, dialect);

    return tpl.arg(columns.join(", "), target, conditionStr);
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

QStringList QueryGenerator::valuesToConditionList(const StrHash<QVariantList>& values, Dialect dialect)
{
    static_qstring(conditionTpl1, "%1 = %2");
    static_qstring(conditionTpl2, "%1 IN (%2)");

    QStringList conditions;
    QStringList conditionValues;
    for (const QString& col : values.keys())
    {
        conditionValues = valueListToSqlList(values[col], dialect);
        conditionValues.removeDuplicates();
        if (conditionValues.size() == 1)
            conditions << conditionTpl1.arg(wrapObjIfNeeded(col, dialect), conditionValues.first());
        else
            conditions << conditionTpl2.arg(wrapObjIfNeeded(col, dialect), conditionValues.join(", "));
    }
    return conditions;
}

QString QueryGenerator::valuesToConditionStr(const StrHash<QVariantList>& values, Dialect dialect)
{
    static_qstring(condTpl, " WHERE %1");

    QStringList conditions = valuesToConditionList(values, dialect);
    QString conditionStr = "";
    if (conditions.size() > 0)
        conditionStr = condTpl.arg(conditions.join(" AND "));

    return conditionStr;
}

QString QueryGenerator::toResultColumnString(const SelectResolver::Column& column, Dialect dialect)
{
    return wrapObjIfNeeded(column.displayName, dialect);
}

QString QueryGenerator::toFullObjectName(const QString& database, const QString& object, Dialect dialect)
{
    static_qstring(tpl, "%1%2");

    QString dbName = "";
    if (!database.isEmpty() && dbName.toLower() != "main")
        dbName = wrapObjIfNeeded(database, dialect);

    if (!dbName.isEmpty())
        dbName.append(".");

    return tpl.arg(dbName, wrapObjIfNeeded(object, dialect));
}

QStringList QueryGenerator::toValueSets(const QStringList& columns, const StrHash<QVariantList> values, Dialect dialect)
{
    QStringList rows;
    QVariantList rowValues;
    QStringList valueList;

    for (int total = values.values().first().size(), i = 0; i < total; i++)
    {
        rowValues.clear();
        for (const QString& col : columns)
            rowValues << values[col][i];

        valueList = valueListToSqlList(rowValues, dialect);
        rows << valueList.join(", ");
    }

    return rows;
}
