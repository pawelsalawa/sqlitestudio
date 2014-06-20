#include "htmlexport.h"
#include "services/pluginmanager.h"
#include "docexport.h"
#include <QTextDocumentWriter>


HtmlExport::HtmlExport()
{
    docExport = dynamic_cast<DocExport*>(PLUGINS->getLoadedPlugin("DocExport"));
}

QString HtmlExport::getFormatName() const
{
    return "HTML";
}

ExportManager::StandardConfigFlags HtmlExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

QString HtmlExport::getExportConfigFormName() const
{
    return QString();
}

void HtmlExport::validateOptions()
{
}

QString HtmlExport::defaultFileExtension() const
{
    return "html";
}

bool HtmlExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns)
{
    return true;
}

bool HtmlExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return true;
}

bool HtmlExport::afterExportQueryResults()
{
    return true;
}

bool HtmlExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, bool databaseExport)
{
    docExport->newDoc();
    docExport->addTable(createTable);

    QTextDocumentWriter writer(output, "HTML");
    bool res = writer.write(docExport->getDocument());
    docExport->cleanupDoc();
    return res;
}

bool HtmlExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, bool databaseExport)
{
    return true;
}

bool HtmlExport::exportTableRow(SqlResultsRowPtr data)
{
    return true;
}

bool HtmlExport::beforeExportDatabase(const QString& database)
{
    return true;
}

bool HtmlExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    return true;
}

bool HtmlExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    return true;
}

bool HtmlExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view)
{
    return true;
}
