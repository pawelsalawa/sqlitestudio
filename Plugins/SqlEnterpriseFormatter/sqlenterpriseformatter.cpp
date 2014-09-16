#include "sqlenterpriseformatter.h"
#include "formatstatement.h"
#include "common/unused.h"
#include "common/global.h"
#include <QDebug>
#include <parser/parser.h>

CFG_DEFINE(SqlEnterpriseFormatterConfig)

SqlEnterpriseFormatter::SqlEnterpriseFormatter()
{
}

QString SqlEnterpriseFormatter::format(SqliteQueryPtr query)
{
    int wrapperIdx = CFG_ADV_FMT.SqlEnterpriseFormatter.Wrappers.get().indexOf(CFG_ADV_FMT.SqlEnterpriseFormatter.PrefferedWrapper.get());
    NameWrapper wrapper = static_cast<NameWrapper>(wrapperIdx);

    FormatStatement *formatStmt = FormatStatement::forQuery(query.data());
    if (!formatStmt)
        return query->detokenize();

    formatStmt->setSelectedWrapper(wrapper);
    QString formatted = formatStmt->format();
    delete formatStmt;

    return formatted;
}

bool SqlEnterpriseFormatter::init()
{
    Q_INIT_RESOURCE(sqlenterpriseformatter);

    static_qstring(query1, "SELECT (2 + 4) AND (3 + 5), 4 NOT IN (SELECT t1.u + t2.y FROM xyz t1 JOIN zxc t2 ON (t1.aaa = t2.aaa)) "
                           "FROM a, (SELECT id FROM table2);");
    static_qstring(query2, "INSERT INTO table1 (id, value1, value2) VALUES (1, (2 + 5), (SELECT id FROM table2));");
    static_qstring(query3, "CREATE TABLE tab (id INTEGER PRIMARY KEY, value1 VARCHAR(6), value2 NUMBER(8,2) NOT NULL DEFAULT 1.0);");
    static_qstring(query4, "CREATE UNIQUE INDEX IF NOT EXISTS dbName.idx1 ON [messages column] (id COLLATE x ASC, lang DESC, description);");

    Parser parser(Dialect::Sqlite3);

    for (const QString& q : {query1, query2, query3, query4})
    {
        if (!parser.parse(q))
        {
            qWarning() << "SqlEnterpriseFormatter preview query parsing error:" << parser.getErrorString();
            continue;
        }
        previewQueries << parser.getQueries().first();
    }

    return GenericPlugin::init();
}

void SqlEnterpriseFormatter::deinit()
{
    Q_CLEANUP_RESOURCE(sqlenterpriseformatter);
}

void SqlEnterpriseFormatter::updatePreview()
{
    QStringList output;
    for (const SqliteQueryPtr& q : previewQueries)
        output << format(q);

    CFG_ADV_FMT.SqlEnterpriseFormatter.PreviewCode.set(output.join("\n\n"));
}

QString Cfg::getNameWrapperStr(NameWrapper wrapper)
{
    return wrapObjName(QObject::tr("name", "example name wrapper"), wrapper);
}

QStringList Cfg::getNameWrapperStrings()
{
    QStringList strings;
    for (NameWrapper nw : {NameWrapper::BRACKET, NameWrapper::DOUBLE_QUOTE, NameWrapper::BACK_QUOTE, NameWrapper::QUOTE})
        strings << wrapObjName(QObject::tr("name", "example name wrapper"), nw);

    return strings;
}

void SqlEnterpriseFormatter::configModified(CfgEntry* key, const QVariant& value)
{
    UNUSED(value);
    if (key->getMain() != CFG_ADV_FMT)
        return;

    updatePreview();
}
