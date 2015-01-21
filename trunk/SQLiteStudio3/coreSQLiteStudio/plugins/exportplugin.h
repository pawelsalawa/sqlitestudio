#ifndef EXPORTPLUGIN_H
#define EXPORTPLUGIN_H

#include "plugin.h"
#include "db/sqlquery.h"
#include "db/queryexecutor.h"
#include "services/exportmanager.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatevirtualtable.h"

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
         * @brief Tells which character encoding use by default in export dialog.
         * @return Name of the encoding.
         *
         * If the plugin doesn't return ExportManager::CODEC in results from standardOptionsToEnable(), then result
         * of this function is ignored and it can return null string.
         */
        virtual QString getDefaultEncoding() const = 0;

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
         * @brief Provides set of flags for additional information that needs to be provided for this plugin.
         * @return OR-ed set of flags.
         *
         * Some plugins might need to know what is a total number of rows that are expected to be
         * exported for each table or query results. Other plugins might want to know
         * what is the maximum number of characters/bytes in each exported table column,
         * so they can calculate column widths when drawing them in the exported files, documents, etc.
         *
         * Those additional information are not provided by default, because they are gathered with extra queries
         * to the database and for huge tables it might cause the table to be exported much longer, even if
         * those information wasn't required by some plugin.
         *
         * See ExportManager::ExportProviderFlags for list of possible flags and what they mean.
         */
        virtual ExportManager::ExportProviderFlags getProviderFlags() const = 0;

        /**
         * @brief Provides config object that holds configuration for exporting.
         * @return Config object, or null if the exporting with this plugin is not configurable.
         */
        virtual CfgMain* getConfig() = 0;

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
        virtual QString getExportConfigFormName() const = 0;

        /**
         * @brief Tells plugin what is going to be the next export mode.
         * @param mode Mode that the next export is going to be performed for.
         *
         * Plugin should remember this and use later when some logic is depended on what the mode is.
         * For example getConfigFormName() (as well as getConfig()) might return different config forms
         * for different modes.
         */
        virtual void setExportMode(ExportManager::ExportMode mode) = 0;

        /**
         * @brief Called when the UI expects any configuration options to be re-validated.
         *
         * When user interacts with the UI in a way that it doesn't change the config values,
         * but it still requires some options to be re-validated, this method is called.
         *
         * It should validate any configuration values defined with CFG_CATEGORY and CFG_ENTRY
         * and post the validation results by calling EXPORT_MANAGER->handleValidationFromPlugin()
         * for every validated CfgEntry.
         *
         * This is also a good idea to connect to the CfgEntry::changed() signal for entries that should be validated
         * and call this method from the slot, so any changes to the configuration values will be
         * immediately validated and reflected on the UI.
         */
        virtual void validateOptions() = 0;

        /**
         * @brief Provides usual file name extension used with this format.
         * @return File name extension (like ".csv").
         *
         * This extension will be automatically appended to file name when user picked file name
         * with file dialog, but the file extension part in the selected file was ommited.
         */
        virtual QString defaultFileExtension() const = 0;

        /**
         * @brief Mime type for when exporting binary format to clipboard.
         * @return  Mime type, like "image/png".
         *
         * Value returned from this method is used to set mime type when the exporting is done into the system clipboard.
         * The clipboard needs mime type to identify what kind of data is in it.
         *
         * See details http://qt-project.org/doc/qt-5/qmimedata.html#setData
         *
         * If the plugin exports just a string, then this method can return QString::null and SqliteStudio will assume
         * that the data is of "text/plain" type.
         */
        virtual QString getMimeType() const = 0;

        /**
         * @brief Tells if the data being exported is a binary or text.
         * @return true for binary data, false for textual data.
         *
         * This is used by the SQLiteStudio to configure output device properly. For example CSV format is textual,
         * but PNG is considered binary.
         */
        virtual bool isBinaryData() const = 0;

        /**
         * @brief Provides common state values before the export process begins.
         * @param db Database that the export will be performed on.
         * @param output Output device to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true for success, or false in case of a fatal error.
         *
         * This is called exactly once before every export process (that is once per each export called by user).
         * Use it to remember database, output device, config for further method calls, or write a file header.
         * This method will be followed by any of *export*() methods from this interface.
         *
         * There's a convenient class GenericExportPlugin, which you can extend instead of ExportPlugin. If you use
         * GenericExportPlugin for a base class of exprt plugin, then this method is already implemented there
         * and it stores all these parameters in protected class members so you can use them in other methods.
         */
        virtual bool initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config) = 0;

        /**
         * @brief Does initial entry for exported query results.
         * @param query Query that was executed to get the results.
         * @param columns Columns returned from the query.
         * @param providedData All data entries requested by the plugin in the return value of getProviderFlags().
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just before actual data entries are exported (with exportQueryResultsRow()).
         * It's called exactly once for single query results export.
         */
        virtual bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                              const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) = 0;

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
         * @brief Prepares for exporting tables from database.
         * @return true for success, or false in case of a fatal error.
         *
         * This is called only for database export. For single table export only exportTable() is called.
         */
        virtual bool beforeExportTables() = 0;

        /**
         * @brief Does initial entry for exported table.
         * @param database "Attach" name of the database that the table belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the table to export.
         * @param columnNames Name of columns in the table, in order they will appear in the rows passed to exportTableRow().
         * @param ddl The DDL of the table.
         * @param createTable Table DDL parsed into an object.
         * @param providedData All data entries requested by the plugin in the return value of getProviderFlags().
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                                 const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) = 0;

        /**
         * @brief Does initial entry for exported virtual table.
         * @param database "Attach" name of the database that the table belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the table to export.
         * @param columnNames Name of columns in the table, in order they will appear in the rows passed to exportTableRow().
         * This will be empty if data is not being exported. This is different than for exportTable(), where columnNames are always present.
         * @param ddl The DDL of the table.
         * @param createTable Table DDL parsed into an object.
         * @param providedData All data entries requested by the plugin in the return value of getProviderFlags().
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl,
                                        SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData) = 0;

        /**
         * @brief Does export entry for a single row of data.
         * @param data Single data row.
         * @return true for success, or false in case of a fatal error.
         *
         * This method will be called only if StandardExportConfig::exportData in initBeforeExport() was true.
         */
        virtual bool exportTableRow(SqlResultsRowPtr data) = 0;

        /**
         * @brief Does final entry for exported table, after its data was exported.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool afterExportTable() = 0;

        /**
         * @brief Does final entries after all tables have been exported.
         * @return true for success, or false in case of a fatal error.
         *
         * This is called only for database export. For single table export only exportTable() is called.
         * After table exporting also an afterExport() is called, so you can use that for any postprocessing.
         */
        virtual bool afterExportTables() = 0;

        /**
         * @brief Does initial entry for the entire database export.
         * @param database Database name (as listed in database list).
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just once, before each database object gets exported.
         * This method will be followed by calls to (in this order): beforeExportTables(), exportTable(), exportVirtualTable(), exportTableRow(), afterExportTable(),
         * afterExportTables(), beforeExportIndexes(), exportIndex(), afterExportIndexes(), beforeExportTriggers(), exportTrigger(), afterExportTriggers(),
         * beforeExportViews(), exportView(), afterExportViews() and afterExportDatabase().
         * Note, that exportTableRow() will be called only if StandardExportConfig::exportData in initBeforeExport() was true.
         */
        virtual bool beforeExportDatabase(const QString& database) = 0;

        /**
         * @brief Prepares for exporting indexes from database.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool beforeExportIndexes() = 0;

        /**
         * @brief Does entire export entry for an index.
         * @param database "Attach" name of the database that the index belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the index to export.
         * @param ddl The DDL of the index.
         * @param createIndex Index DDL parsed into an object.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for index export.
         */
        virtual bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex) = 0;

        /**
         * @brief Does final entries after all indexes have been exported.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool afterExportIndexes() = 0;

        /**
         * @brief Prepares for exporting triggers from database.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool beforeExportTriggers() = 0;

        /**
         * @brief Does entire export entry for an trigger.
         * @param database "Attach" name of the database that the trigger belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the trigger to export.
         * @param ddl The DDL of the trigger.
         * @param createTrigger Trigger DDL parsed into an object.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for trigger export.
         */
        virtual bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger) = 0;

        /**
         * @brief Does final entries after all triggers have been exported.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool afterExportTriggers() = 0;

        /**
         * @brief Prepares for exporting views from database.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool beforeExportViews() = 0;

        /**
         * @brief Does entire export entry for an view.
         * @param database "Attach" name of the database that the view belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the trigger to view.
         * @param ddl The DDL of the view.
         * @param createView View DDL parsed into an object.
         * @return true for success, or false in case of a fatal error.
         *
         * This is the only method called for view export.
         */
        virtual bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view) = 0;

        /**
         * @brief Does final entries after all views have been exported.
         * @return true for success, or false in case of a fatal error.
         */
        virtual bool afterExportViews() = 0;

        /**
         * @brief Does final entry for the entire database export.
         * @return true for success, or false in case of a fatal error.
         *
         * It's called just once, after all database object get exported.
         */
        virtual bool afterExportDatabase() = 0;

        /**
         * @brief Does final entry for any export process.
         * @return true for success, or false in case of a fatal error.
         *
         * This is similar to afterExportDatabase() when the export mode is database,
         * but this is called at the end for any export mode, not only for database export.
         *
         * Use it to write a footer, or anything like that.
         */
        virtual bool afterExport() = 0;

        /**
         * @brief Called after every export, even failed one.
         *
         * Implementation of this method should cleanup any resources used during each single export process.
         * This method is guaranteed to be executed, no matter if export was successful or not.
         */
        virtual void cleanupAfterExport() = 0;
};

#endif // EXPORTPLUGIN_H
