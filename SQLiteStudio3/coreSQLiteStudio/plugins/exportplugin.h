#ifndef EXPORTPLUGIN_H
#define EXPORTPLUGIN_H

#include "plugin.h"
#include "db/sqlresults.h"
#include "db/queryexecutor.h"
#include "services/exportmanager.h"

class CfgMain;

class ExportPlugin : virtual public Plugin
{
    public:
        /**
         * @brief Provides name of the format that the plugin exports to.
         * @return Format name that will be displayed on UI.
         *
         * Format must be a single word name.
         */
        virtual QString getFormatName() const = 0;

        /**
         * @brief Tells what standard exporting options should be displayed to the user on UI.
         * @return OR-ed set of option enum values.
         */
        virtual ExportManager::StandardConfigFlags standardOptionsToEnable() const = 0;

        /**
         * @brief Provides set of modes supported by this export plugin.
         * @return OR-ed set of supported modes.
         *
         * Some export plugins might not support some of exporting modes. For example CSV export plugin
         * will not support DATABASE exporting, because CSV cannot represent schema of the database.
         *
         * If a plugin doesn't return some mode in this method, then that plugin will be excluded
         * from list of available formats to export, when user requests to export in this mode.
         */
        virtual ExportManager::ExportModes getSupportedModes() const = 0;

        /**
         * @brief Provides config object that holds configuration for exporting.
         * @return Config object, or null if the exporting with this plugin is not configurable.
         */
        virtual CfgMain* getConfig() const = 0;

        /**
         * @brief Provides name of the form to use for configuration of exporting in given mode.
         * @param mode Mode for which the form is requested for.
         * @return Name of the form (toplevel QWidget in the ui file).
         *
         * If exporting with this plugin is not configurable (i.e. getConfig() returns null),
         * then this method is not even called, so it can return anything, just to satisfy method
         * return type. In that case good idea is to always return QString::null.
         *
         * @see FormManager
         */
        virtual QString getConfigFormName(ExportManager::ExportMode mode) const = 0;

        /**
         * @brief Provides usual file name extension used with this format.
         * @return File name extension (like ".csv").
         *
         * This extension will be automatically appended to file name when user picked file name
         * with file dialog, but the file extension part in the selected file was ommited.
         */
        virtual QString defaultFileExtension() const = 0;

        /**
         * @brief Exports results of the query.
         * @param Db Database that the query was executed on.
         * @param query Query that was executed to get the results.
         * @param results Results to export.
         * @param output Output device to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual bool exportQueryResults(Db* db, const QString& query, SqlResultsPtr results, QList<QueryExecutor::ResultColumnPtr>& columns, QIODevice* output,
                                        const ExportManager::StandardExportConfig& config) = 0;

        /**
         * @brief Exports table DDL and it's data.
         * @param db Database that the table belongs to.
         * @param database "Attach" name of the database that the table belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the table to export.
         * @param ddl The DDL of the table. If
         * @param data Table data (will be null pointer if data exporting was disabled on UI by user, which is indicated in @p config).
         * @param output Output device to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Data should be exported only if StandardExportConfig::exportData is true.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual bool exportTable(Db* db, const QString& database, const QString& table, const QString& ddl, SqlResultsPtr data, QIODevice* output,
                                 const ExportManager::StandardExportConfig& config) = 0;

        /**
         * @brief Exports database (all its objects and data from tables).
         * @param db Database being exported.
         * @param objectsToExport Objects from the database to export
         * @param output Output device to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Data from tables should be exported only if StandardExportConfig::exportData is true.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual bool exportDatabase(Db* db, const QList<ExportManager::ExportObject>& objectsToExport, QIODevice* output,
                                    const ExportManager::StandardExportConfig& config) = 0;
};

#endif // EXPORTPLUGIN_H
