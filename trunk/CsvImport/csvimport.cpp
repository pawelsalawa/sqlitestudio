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
    stream->setCodec(config.codec.toLatin1().data());

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
    QString line = stream->readLine();
    while (line.trimmed().isEmpty() && !stream->atEnd())
        line = stream->readLine();

    if (line.trimmed().isEmpty())
    {
        notifyError(tr("Could not find any data in the file %1.").arg(file->fileName()));
        return false;
    }

    QStringList deserialized = CsvSerializer::deserialize(line.trimmed(), csvFormat).first();
    if (cfg.CsvImport.FirstRowAsColumns.get())
    {
        columnNames = deserialized;
    }
    else
    {
        static const QString colTmp = QStringLiteral("column%1");
        columnNames.clear();
        for (int i = 1, total = deserialized.size(); i <= total; ++i)
            columnNames << colTmp.arg(i);

        stream->seek(0);
    }

    return true;
}

void CsvImport::defineCsvFormat()
{
    csvFormat = CsvFormat();
    csvFormat.rowSeparator = '\n';

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
    QString line = stream->readLine();
    if (line.isNull())
        return QList<QVariant>();

    QList<QVariant> values;
    QList<QStringList> deserialized = CsvSerializer::deserialize(line, csvFormat);
    if (deserialized.size() > 0)
    {
        if (cfg.CsvImport.NullValues.get())
        {
            QString nullVal = cfg.CsvImport.NullValueString.get();
            for (const QString& val : deserialized.first())
            {
                if (val == nullVal)
                    values << QVariant(QVariant::String);
                else
                    values << val;
            }
        }
        else
        {
            for (const QString& val : deserialized.first())
                values << val;
        }
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

void CsvImport::validateOptions()
{
    if (cfg.CsvImport.Separator.get() >= 4)
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.CustomSeparator, true, true);

        bool valid = !cfg.CsvImport.CustomSeparator.get().isEmpty();
        IMPORT_MANAGER->handleValidationFromPlugin(valid, cfg.CsvImport.CustomSeparator, tr("Enter the custom separator character."));
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.CustomSeparator, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, cfg.CsvImport.CustomSeparator);
    }

    if (cfg.CsvImport.NullValues.get())
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.NullValueString, true, true);

        bool valid = !cfg.CsvImport.NullValueString.get().isEmpty();
        IMPORT_MANAGER->handleValidationFromPlugin(valid, cfg.CsvImport.NullValueString, tr("Enter the value that will be interpreted as a NULL."));
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.CsvImport.NullValueString, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, cfg.CsvImport.NullValueString);
    }
}

QString CsvImport::getFileFilter() const
{
    return tr("CSV files (*.csv);;Text files (*.txt);;All files (*)");
}
