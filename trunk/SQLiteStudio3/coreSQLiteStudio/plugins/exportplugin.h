#ifndef EXPORTPLUGIN_H
#define EXPORTPLUGIN_H

#include "plugin.h"
#include "db/sqlresults.h"
#include "db/queryexecutor.h"

class CfgMain;

class ExportPlugin : virtual public Plugin
{
    public:
        // TODO move to exporter file
        enum class ExportMode
        {
            DATABASE,
            TABLE,
            RESULTS
        };

        /**
         * @brief Standard configuration for all exporting processes.
         *
         * Object of this type is passed to all exporting processes.
         * It is configured with standard UI config for export.
         */
        struct StandardExportConfig
        {
            /**
             * @brief Text encoding.
             *
             * Always one of QTextCodec::availableCodecs().
             */
            QString codec;

            /**
             * @brief Name of the file that the export being done to.
             *
             * This is provided just for information to the export process,
             * but the plugin should use data stream provided to each called export method,
             * instead of opening the file from this name.
             *
             * It will be null string if exporting is not performed into a file, but somewhere else
             * (for example into a clipboard).
             */
            QString outputFileName;

            /**
             * @brief Indicates exporting to clipboard.
             *
             * This is just for an information, like outputFileName. Exporting methods will
             * already have stream prepared for exporting to clipboard.
             *
             * Default is false.
             */
            bool intoClipboard = false;

            /**
             * @brief When exporting table or database, this indicates if table data should also be exported.
             *
             * Default is true.
             */
            bool exportData = true;

            /**
             * @brief Indicates whether DDLs will be pre-formatted by SQL formatter when passed to export methods
             * for exporting tables and databases.
             *
             * Default is true.
             */
            bool formatDdl = true;
        };

        /**
         * @brief Standard export configuration options to be displayed on UI.
         *
         * Each of enum represents single property of StandardExportConfig which will be
         * available on UI to configure.
         */
        enum StandardConfigOptions
        {
            CODEC,              /**< Text encoding (see StandardExportConfig::codec). */
            FILENAME,           /**< File name to export to. This will show file entry with a 'Browse' button. */
            CLIPBOARD,          /**< 'Export to clipboard' as an option to this export. Only valid if FILENAME is also present. */
            OBJECTS_SELECTION,  /**< Shows option to select database objects to export in case of exporting a database. */
            EXPORT_DATA,        /**< Shows checkbox to decide if table data should be exported in case of table and database exports. */
            FORMAT_DDL          /**< Shows checkbox to decide if DDL should be formatted for exporting with the current SQL formatter. */
        };

        // TODO move to exporter file
        struct ExportObject
        {
            enum Type
            {
                TABLE,
                INDEX,
                TRIGGER,
                VIEW
            };

            QString database;
            QString name;
            SqlResultsPtr data;
        };

        /**
         * @brief Tells what standard exporting options should be displayed to the user on UI.
         * @return OR-ed set of option enum values.
         */
        virtual StandardConfigOptions standardOptionsToEnable() const = 0;

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
        virtual QString getConfigFormName(ExportMode mode) const = 0;

        /**
         * @brief Exports results of the query.
         * @param query Query that was executed to get the results.
         * @param results Results to export.
         * @param output Output stream to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual bool exportQueryResults(const QString& query, SqlResultsPtr results, QList<QueryExecutor::ResultColumnPtr>& columns, QDataStream& output,
                                        const StandardExportConfig& config) = 0;

        /**
         * @brief Exports table DDL and it's data.
         * @param db Database that the table belongs to.
         * @param database "Attach" name of the database that the table belongs to. Can be "main", "temp", or any attach name.
         * @param table Name of the table to export.
         * @param ddl The DDL of the table. If
         * @param data Table data (will be null pointer if data exporting was disabled on UI by user, which is indicated in @p config).
         * @param output Output stream to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Data should be exported only if StandardExportConfig::exportData is true.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual bool exportTable(Db* db, const QString& database, const QString& table, const QString& ddl, SqlResultsPtr data, QDataStream& output,
                                 const StandardExportConfig& config) = 0;

        /**
         * @brief When exporting database, this tells which objects from the database to export.
         *
         */

        /**
         * @brief Exports database (all its objects and data from tables).
         * @param db Database being exported.
         * @param objectsToExport Objects from the database to export
         * @param output Output stream to write exporting data to.
         * @param config Common exporting configuration, like file name, codec, etc.
         * @return true if the export was successful, or false in case of fatal error.
         *
         * Data from tables should be exported only if StandardExportConfig::exportData is true.
         *
         * Method should report any warnings, error messages, etc through NotifyManager, that is by using notifyError() and its family methods.
         */
        virtual vool exportDatabase(Db* db, const QList<ExportObject>& objectsToExport, QDataStream& output, const StandardExportConfig& config);
};

#endif // EXPORTPLUGIN_H
