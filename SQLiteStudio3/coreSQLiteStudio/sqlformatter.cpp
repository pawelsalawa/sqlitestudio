#include "sqlformatter.h"
#include "parser/parser.h"
#include "plugins/sqlformatterplugin.h"
#include "services/pluginmanager.h"
#include <QDebug>

QString SqlFormatter::format(const QString &sql, Dialect dialect)
{
    Parser parser(dialect);
    if (!parser.parse(sql))
    {
        qWarning() << "Could not parse SQL in order to format it. The SQL was:" << sql;
        return sql;
    }

    QStringList formattedQueries;
    foreach (SqliteQueryPtr query, parser.getQueries())
        formattedQueries << format(query);

    return formattedQueries.join("\n");
}

QString SqlFormatter::format(SqliteQueryPtr query)
{
    if (!currentFormatter)
    {
        qWarning() << "No formatter plugin defined for SqlFormatter.";
        return query->detokenize();
    }

    return currentFormatter->format(query);
}

void SqlFormatter::setFormatter(SqlFormatterPlugin *formatterPlugin)
{
    currentFormatter = formatterPlugin;
}

SqlFormatterPlugin* SqlFormatter::getFormatter()
{
    return currentFormatter;
}
