#include "querygenerator.h"
#include "common/global.h"
#include "common/utils_sql.h"
#include "db/db.h"

QueryGenerator::QueryGenerator()
{

}

QString QueryGenerator::generateSelectFromTable(Db* db, const QString& table, const QHash<QString, QVariantList> values)
{
    return generateSelectFromTable(db, QString(), table, values);
}

QString QueryGenerator::generateSelectFromTable(Db* db, const QString& database, const QString& table, const QHash<QString, QVariantList> values)
{
    SchemaResolver resolver(db);
    QStringList columns = resolver.getTableColumns(database, table);
    return generateSelectFromTableOrView(db, database, table, columns, values);
}

QString QueryGenerator::generateSelectFromView(Db* db, const QString& view, const QHash<QString, QVariantList> values)
{
    return generateSelectFromView(db, QString(), view, values);
}

QString QueryGenerator::generateSelectFromView(Db* db, const QString& database, const QString& view, const QHash<QString, QVariantList> values)
{
    SchemaResolver resolver(db);
    QStringList columns = resolver.getViewColumns(database, view);
    return generateSelectFromTableOrView(db, database, view, columns, values);
}

QString QueryGenerator::generateSelectFromSelect(Db* db, const QString& initialSelect, const QHash<QString, QVariantList> values, const BiStrHash& dbNameToAttach)
{
    static_qstring(tpl, "SELECT %1 FROM (%2)%3");

    Dialect dialect = db->getDialect();

    SelectResolver resolver(db, initialSelect, dbNameToAttach);
    QList<SelectResolver::Column> columns = resolver.resolveColumnsFromFirstCore();

    QStringList resCols;
    for (const SelectResolver::Column& col : columns)
        resCols << toResultColumnString(col, dialect);

    QString conditionStr = valuesToConditionStr(values, dialect);
    return tpl.arg(resCols.join(", "), initialSelect, conditionStr);
}

QString QueryGenerator::generateSelectFromTableOrView(Db* db, const QString& database, const QString& tableOrView, const QStringList& columns, const QHash<QString, QVariantList> values)
{
    static_qstring(tpl, "SELECT %1 FROM %2%3%4");
    static_qstring(tableTpl, "%1.%2 AS %3");

    Dialect dialect = db->getDialect();

    QString dbName = "";
    if (!database.isEmpty() && dbName.toLower() != "main")
        dbName = wrapObjIfNeeded(database, dialect);

    if (!dbName.isEmpty())
        dbName.append(".");

    QString conditionStr = valuesToConditionStr(values, dialect);
    return tpl.arg(columns.join(", "), dbName, tableOrView, conditionStr);
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

QStringList QueryGenerator::valuesToConditionList(const QHash<QString, QVariantList>& values, Dialect dialect)
{
    static_qstring(conditionTpl1, "%1 = %2");
    static_qstring(conditionTpl2, "%1 IN (%2)");

    QStringList conditions;
    QStringList conditionValues;
    for (const QString& col : values.keys())
    {
        conditionValues = valueListToSqlList(values[col], dialect);
        if (conditionValues.size() == 1)
            conditions << conditionTpl1.arg(wrapObjIfNeeded(col, dialect), conditionValues.first());
        else
            conditions << conditionTpl2.arg(wrapObjIfNeeded(col, dialect), conditionValues.join(", "));
    }
    return conditions;
}

QString QueryGenerator::valuesToConditionStr(const QHash<QString, QVariantList>& values, Dialect dialect)
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
