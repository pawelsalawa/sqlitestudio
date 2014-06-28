#include "pdfexport.h"
#include <QTextDocument>
#include <QPainter>
#include <QPdfWriter>


QString PdfExport::getFormatName() const
{
    return "PDF";
}

ExportManager::StandardConfigFlags PdfExport::standardOptionsToEnable() const
{
    return ExportManager::CODEC;
}

void PdfExport::validateOptions()
{
}

QString PdfExport::defaultFileExtension() const
{
    return "pdf";
}

bool PdfExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns)
{
    return true;
}

bool PdfExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    return true;
}

bool PdfExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable)
{
    return true;
}

bool PdfExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable)
{
    return true;
}

bool PdfExport::exportTableRow(SqlResultsRowPtr data)
{
    return true;
}

bool PdfExport::beforeExportDatabase(const QString& database)
{
    return true;
}

bool PdfExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    return true;
}

bool PdfExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    return true;
}

bool PdfExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view)
{
    return true;
}
