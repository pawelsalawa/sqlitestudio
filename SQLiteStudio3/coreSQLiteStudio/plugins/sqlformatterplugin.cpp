#include "sqlformatterplugin.h"
#include "parser/parser.h"
#include "db/db.h"
#include "common/unused.h"
#include <QDebug>

QString SqlFormatterPlugin::format(const QString& code, Db* contextDb)
{
    UNUSED(contextDb);
    Parser parser;
    if (!parser.parse(code))
    {
        qWarning() << "Could not parse SQL in order to format it. The SQL was:" << code;
        return code;
    }

    QStringList formattedQueries;
    for (SqliteQueryPtr query : parser.getQueries())
        formattedQueries << format(query);

    return formattedQueries.join("\n");
}

QString SqlFormatterPlugin::getLanguage() const
{
    return "sql";
}
