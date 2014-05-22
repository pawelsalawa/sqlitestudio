#include "csvimport.h"
#include "services/importmanager.h"
#include "sqlitestudio.h"
#include <QVariant>
#include <QFile>
#include <QTextStream>

CFG_DEFINE_RUNTIME(CsvImportConfig)

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
//    file = new QFile(config.inputFileName);

    return false;
}

void CsvImport::afterImport()
{
    safe_delete(file);
}

QList<ImportPlugin::ColumnDefinition> CsvImport::getColumns() const
{
    return QList<ImportPlugin::ColumnDefinition>();
}

QList<QVariant> CsvImport::next()
{
    return QList<QVariant>();
}

CfgMain* CsvImport::getConfig() const
{
    return &CSV_IMPORT_CFG;
}

QString CsvImport::getImportConfigFormName() const
{
    return "csvImportOptions";
}

void CsvImport::validateOptions()
{
    if (CSV_IMPORT_CFG.CsvImport.Separator.get() >= 4)
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(CSV_IMPORT_CFG.CsvImport.CustomSeparator, true, true);

        bool valid = !CSV_IMPORT_CFG.CsvImport.CustomSeparator.get().isEmpty();
        IMPORT_MANAGER->handleValidationFromPlugin(valid, CSV_IMPORT_CFG.CsvImport.CustomSeparator, tr("Enter the custom separator character."));
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(CSV_IMPORT_CFG.CsvImport.CustomSeparator, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, CSV_IMPORT_CFG.CsvImport.CustomSeparator);
    }

    if (CSV_IMPORT_CFG.CsvImport.NullValues.get())
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(CSV_IMPORT_CFG.CsvImport.NullValueString, true, true);

        bool valid = !CSV_IMPORT_CFG.CsvImport.NullValueString.get().isEmpty();
        IMPORT_MANAGER->handleValidationFromPlugin(valid, CSV_IMPORT_CFG.CsvImport.NullValueString, tr("Enter the value that will be interpreted as a NULL."));
    }
    else
    {
        IMPORT_MANAGER->updateVisibilityAndEnabled(CSV_IMPORT_CFG.CsvImport.NullValueString, true, false);
        IMPORT_MANAGER->handleValidationFromPlugin(true, CSV_IMPORT_CFG.CsvImport.NullValueString);
    }
}

QString CsvImport::getFileFilter() const
{
    return tr("CSV files (*.csv);;Text files (*.txt);;All files (*)");
}
