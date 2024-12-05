#include "csvimport.h"
#include "services/importmanager.h"
#include "sqlitestudio.h"
#include "services/notifymanager.h"
#include <QVariant>
#include <QFile>
#include <QTextStream>

CsvImport::CsvImport()
{
}

QString CsvImport::getDataSourceTypeName() const
{
    return "CSV";
}

ImportManager::StandardConfigFlags CsvImport::standardOptionsToEnable() const
{
    return ImportManager::CODEC|ImportManager::FILE_NAME;
}

bool CsvImport::beforeImport(const ImportManager::StandardImportConfig& config)
{
    defineCsvFormat();

    file = new QFile(config.inputFileName);
    if (!file->open(QFile::ReadOnly) || !file->isReadable())
    {
        notifyError(tr("Cannot read file %1").arg(config.inputFileName));
        safe_delete(file);
        return false;
    }

    stream = new QTextStream(file);
    stream->setEncoding(textEncodingForName(config.codec));

    if (!extractColumns())
    {
        safe_delete(stream);
        safe_delete(file);
        return false;
    }

    return true;
}

void CsvImport::afterImport()
{
    safe_delete(stream);
    safe_delete(file);
}

bool CsvImport::extractColumns()
{
    QStringList deserializedEntry = CsvSerializer::deserializeOneEntry(*stream, csvFormat);
    while (deserializedEntry.isEmpty() && !stream->atEnd())
        deserializedEntry = CsvSerializer::deserializeOneEntry(*stream, csvFormat);

    if (deserializedEntry.isEmpty())
    {
        notifyError(tr("Could not find any data in the file %1.").arg(file->fileName()));
        return false;
    }

    if (cfg.CsvImport.FirstRowAsColumns.get())
    {
        columnNames = deserializedEntry;
    }
    else
    {
        static const QString colTmp = QStringLiteral("column%1");
        columnNames.clear();
        for (int i = 1, total = deserializedEntry.size(); i <= total; ++i)
            columnNames << colTmp.arg(i);

        stream->seek(0);
    }

    return true;
}

void CsvImport::defineCsvFormat()
{
    csvFormat = CsvFormat();
    csvFormat.rowSeparators = QStringList({"\r\n", "\n", "\r"});
    csvFormat.multipleRowSeparators = true;
    csvFormat.strictRowSeparator = true;
    csvFormat.quotationMark = cfg.CsvImport.QuotationMark.get();

    switch (cfg.CsvImport.Separator.get())
    {
        case 0:
            csvFormat.columnSeparator = ',';
            break;
        case 1:
            csvFormat.columnSeparator = ';';
            break;
        case 2:
            csvFormat.columnSeparator = '\t';
            break;
        case 3:
            csvFormat.columnSeparator = ' ';
            break;
        default:
            csvFormat.columnSeparator = cfg.CsvImport.CustomSeparator.get();
            break;
    }

    csvFormat.calculateSeparatorMaxLengths();
}

QList<ImportPlugin::ColumnDefinition> CsvImport::getColumns() const
{
    QList<ImportPlugin::ColumnDefinition> columnList;
    for (const QString& colName : columnNames)
        columnList << ImportPlugin::ColumnDefinition(colName, QString());

    return columnList;
}

QList<QVariant> CsvImport::next()
{
    QStringList deserializedEntry = CsvSerializer::deserializeOneEntry(*stream, csvFormat);

    QList<QVariant> values;
    if (deserializedEntry.isEmpty())
        return values;

    if (cfg.CsvImport.NullValues.get())
    {
        QString nullVal = cfg.CsvImport.NullValueString.get();
        for (const QString& val : deserializedEntry)
        {
            if (val == nullVal)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                values << QVariant(QVariant::String);
#else
                values << QVariant(QMetaType::fromType<QString>());
#endif
            else
                values << val;
        }
    }
    else
    {
        for (const QString& val : deserializedEntry)
            values << val;
    }

    return values;
}

CfgMain* CsvImport::getConfig()
{
    return &cfg;
}

QString CsvImport::getImportConfigFormName() const
{
    return "csvImportOptions";
}

bool CsvImport::validateOptions()
{
    bool isValid = true;
    if (cfg.CsvImport.Separator.get() >= 4)
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.CustomSeparator, true, true);

        bool valid = !cfg.CsvImport.CustomSeparator.get().isEmpty();
        IMPORT_MANAGER->handleValidationFromPlugin(valid, cfg.CsvImport.CustomSeparator, tr("Enter the custom separator character."));
        isValid &= valid;
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.CustomSeparator, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, cfg.CsvImport.CustomSeparator);
    }

    if (cfg.CsvImport.NullValues.get())
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.NullValueString, true, true);
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.NullValueString, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, cfg.CsvImport.NullValueString);
    }
    return isValid;
}

QString CsvImport::getFileFilter() const
{
    return tr("CSV files (*.csv);;Text files (*.txt);;All files (*)");
}

bool CsvImport::init()
{
    SQLS_INIT_RESOURCE(csvimport);
    return GenericPlugin::init();
}

void CsvImport::deinit()
{
    SQLS_CLEANUP_RESOURCE(csvimport);
}
