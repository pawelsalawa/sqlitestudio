#include "sqlformatterplugin.h"
#include "parser/parser.h"
#include "db/db.h"
#include <QDebug>

QString SqlFormatterPlugin::format(const QString& code, Db* contextDb)
{
    Dialect dialect = Dialect::Sqlite3;
    if (contextDb && contextDb->isValid())
        contextDb->getDialect();

    Parser parser(dialect);
    if (!parser.parse(code))
    {
        qWarning() << "Could not parse SQL in order to format it. The SQL was:" << code;
        return code;
    }

    QStringList formattedQueries;
    foreach (SqliteQueryPtr query, parser.getQueries())
        formattedQueries << format(query);

    return formattedQueries.join("\n");
}

QString SqlFormatterPlugin::getLanguage() const
{
    return "sql";
}
