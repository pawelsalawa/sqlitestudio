#ifndef EXPORTPLUGIN_H
#define EXPORTPLUGIN_H

#include "plugin.h"
#include "db/sqlresults.h"
#include "db/queryexecutor.h"
#include "services/exportmanager.h"

class CfgMain;

/**
 * @brief Provides support for particular export format.
 *
 * All export methods in this class should report any warnings, error messages, etc through the NotifyManager,
 * that is by using notifyError() and its family methods.
 */
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
         * @brief Provides common state values before the export process begins.
         * @param db Database that the export will be performed on.
         * @param output Output device to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         *
         * This is called exactly once before every export process (that is once per each export called by user).
         * Use it to remember database, output device and config for further method calls. This method will be
         * followed by any of *export*() methods from this interface.
         */
        virtual void initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config) = 0;

        /**
         * @brief Does initial entry for exported query results.
         * @param query Query that was executed to get the results.
         * @param columns Columns returned from the query.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just before actual data entries are exported (with exportQueryResultsRow()).
         * It's called exactly once for single query results export.
         */
        virtual bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns) = 0;

        /**
         * @brief Does export entry for a single row of data.
         * @param row Single data row.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called for each data row returned from the query.
         */
        virtual bool exportQueryResultsRow(SqlResultsRowPtr row) = 0;

        /**
         * @brief Does final entry for exported query results.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called once after all data from the query was exported.
         */
        virtual bool afterExportQueryResults() = 0;

        /**
         * @brief Does initial entry for exported table.
         * @param database "Attach" name of the database that the table belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the table to export.
         * @param ddl The DDL of the table.
         * @param databaseExport true if this table export is a part of exporting the entire databasase,
         * false if it's for exporting just this single table.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool beforeExportTable(const QString& database, const QString& table, const QString& ddl, bool databaseExport) = 0;

        /**
         * @brief Does export entry for a single row of data.
         * @param data Single data row.
         * @return true for success, or false in case of a fatal error.
         *
         * This method will be called only if StandardExportConfig::exportData in beforeExportTable() was true.
         */
        virtual bool exportTableRow(SqlResultsRowPtr data) = 0;

        /**
         * @brief Does final entry fot exported table.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool afterExportTable() = 0;

        /**
         * @brief Does initial entry for the entire database export.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just once, before all database object get exported.
         * This method will be followed by calls to: beforeExportTable(), exportTableRow(), afterExportTable(), exportIndex(),
         * exportTrigger() and exportView().
         * Note, that exportTableRow() will be called only if StandardExportConfig::exportData in beforeExportTable() was true.
         */
        virtual bool beforeExportDatabase() = 0;

        /**
         * @brief Does entire export entry for an index.
         * @param database "Attach" name of the database that the index belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the index to export.
         * @param ddl The DDL of the index.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for index export.
         */
        virtual bool exportIndex(const QString& database, const QString& name, const QString& ddl) = 0;

        /**
         * @brief Does entire export entry for an trigger.
         * @param database "Attach" name of the database that the trigger belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the trigger to export.
         * @param ddl The DDL of the trigger.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for trigger export.
         */
        virtual bool exportTrigger(const QString& database, const QString& name, const QString& ddl) = 0;

        /**
         * @brief Does entire export entry for an view.
         * @param database "Attach" name of the database that the view belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the trigger to view.
         * @param ddl The DDL of the view.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for view export.
         */
        virtual bool exportView(const QString& database, const QString& name, const QString& ddl) = 0;

        /**
         * @brief Does final entry for the entire database export.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just once, after all database object get exported.
         */
        virtual bool afterExportDatabase() = 0;
};

#endif // EXPORTPLUGIN_H
