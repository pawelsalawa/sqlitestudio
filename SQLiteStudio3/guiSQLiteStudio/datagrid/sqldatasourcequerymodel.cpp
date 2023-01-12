#include "sqldatasourcequerymodel.h"
#include "common/utils_sql.h"

SqlDataSourceQueryModel::SqlDataSourceQueryModel(QObject *parent) :
    SqlQueryModel(parent)
{
}

QString SqlDataSourceQueryModel::getDatabase() const
{
    return database;
}

SqlQueryModel::Features SqlDataSourceQueryModel::features() const
{
    return FILTERING;
}

void SqlDataSourceQueryModel::updateTablesInUse(const QString& inUse)
{
    QString dbName = database;
    if (database.toLower() == "main" || database.isEmpty())
        dbName = QString();

    tablesInUse.clear();
    tablesInUse << DbAndTable(db, dbName, inUse);
}

void SqlDataSourceQueryModel::applyFilter(const QString& value, FilterValueProcessor valueProc)
{
//    static_qstring(sql, "SELECT * FROM %1 WHERE %2");
    if (value.isEmpty())
    {
        resetFilter();
        return;
    }

    QStringList conditions;
    for (SqlQueryModelColumnPtr& column : columns)
        conditions << wrapObjIfNeeded(column->getAliasedName())+" "+valueProc(value);

//    setQuery(sql.arg(getDataSource(), conditions.join(" OR ")));
    queryExecutor->setFilters(conditions.join(" OR "));
    executeQuery();
}

void SqlDataSourceQueryModel::applyFilter(const QStringList& values, FilterValueProcessor valueProc)
{
//    static_qstring(sql, "SELECT * FROM %1 WHERE %2");
    if (values.isEmpty())
    {
        resetFilter();
        return;
    }

    if (values.size() != columns.size())
    {
        qCritical() << "Asked to per-column filter, but number columns"
                    << columns.size() << "is different than number of values" << values.size();
        return;
    }

    QStringList conditions;
    for (int i = 0, total = columns.size(); i < total; ++i)
    {
        if (values[i].isEmpty())
            continue;

        conditions << wrapObjIfNeeded(columns[i]->getAliasedName())+" "+valueProc(values[i]);
    }

//    setQuery(sql.arg(getDataSource(), conditions.join(" AND ")));
    queryExecutor->setFilters(conditions.join(" AND "));
    executeQuery();
}

QString SqlDataSourceQueryModel::stringFilterValueProcessor(const QString& value)
{
    static_qstring(pattern, "LIKE '%%1%'");
    return pattern.arg(escapeString(value));
}

QString SqlDataSourceQueryModel::strictFilterValueProcessor(const QString& value)
{
    static_qstring(pattern, "= '%1'");
    return pattern.arg(escapeString(value));
}

QString SqlDataSourceQueryModel::regExpFilterValueProcessor(const QString& value)
{
    static_qstring(pattern, "REGEXP '%1'");
    return pattern.arg(escapeString(value));
}

void SqlDataSourceQueryModel::applySqlFilter(const QString& value)
{
    if (value.isEmpty())
    {
        resetFilter();
        return;
    }

//    setQuery("SELECT * FROM "+getDataSource()+" WHERE "+value);
    queryExecutor->setFilters(value);
    executeQuery();
}

void SqlDataSourceQueryModel::applyStringFilter(const QString& value)
{
    applyFilter(value, &stringFilterValueProcessor);
}

void SqlDataSourceQueryModel::applyStringFilter(const QStringList& values)
{
    applyFilter(values, &stringFilterValueProcessor);
}

void SqlDataSourceQueryModel::applyRegExpFilter(const QString& value)
{
    applyFilter(value, &regExpFilterValueProcessor);
}

void SqlDataSourceQueryModel::applyRegExpFilter(const QStringList& values)
{
    applyFilter(values, &regExpFilterValueProcessor);
}

void SqlDataSourceQueryModel::applyStrictFilter(const QString& value)
{
    applyFilter(value, &strictFilterValueProcessor);
}

void SqlDataSourceQueryModel::applyStrictFilter(const QStringList& values)
{
    applyFilter(values, &strictFilterValueProcessor);
}

void SqlDataSourceQueryModel::resetFilter()
{
//    setQuery("SELECT * FROM "+getDataSource());
    queryExecutor->setFilters(QString());
    executeQuery();
}

QString SqlDataSourceQueryModel::getDatabasePrefix()
{
    if (database.isNull())
        return ""; // not "main.", because the "main." doesn't work for TEMP tables, such as sqlite_temp_master

    return wrapObjIfNeeded(database) + ".";
}

QString SqlDataSourceQueryModel::getDataSource()
{
    return QString();
}
