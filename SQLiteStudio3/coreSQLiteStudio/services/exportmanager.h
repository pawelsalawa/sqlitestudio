#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include "db/sqlresults.h"
#include "db/db.h"
#include <QObject>

class ExportPlugin;
class QueryExecutor;

/**
 * @brief Provides database exporting capabilities.
 *
 * ExportManager is not thread-safe. Use it from single thread.
 */
class ExportManager : public QObject
{
        Q_OBJECT
    public:
        enum class ExportMode
        {
            DATABASE,
            TABLE,
            RESULTS
        };

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


        explicit ExportManager(QObject *parent = 0);
        ~ExportManager();

        QStringList getAvailableFormats() const;
        ExportPlugin* getPluginForFormat(const QString& formatName) const;

        /**
         * @brief Configures export service for export.
         * @param format Format to be used in upcoming export.
         * @param config Standard configuration options to be used in upcoming export.
         *
         * ExportManager takes ownership of the config object.
         *
         * Call this method just befor any of export*() methods is called to prepare ExportManager for upcoming export process.
         * Otherwise the export process will use settings from last configure() call.
         *
         * If any export is already in progress, this method reports error in logs and does nothing.
         * If plugin for specified format cannot be found, then this method reports warning in logs and does nothing.
         */
        void configure(const QString& format, StandardConfigOptions* config);
        bool isExportInProgress() const;
        void exportQueryResults(Db* db, const QString& query);
        void exportTable(Db* db, const QString& database, const QString& table);
        void exportDatabase(Db* db);

    private:
        void invalidFormat(const QString& format);
        bool checkInitialConditions();
        void processExportQueryResults(ExportPlugin* plugin, Db* db, const QString& query, SqlResultsPtr results);
        QIODevice* getOutputStream();

        bool exportInProgress = false;
        QueryExecutor* executor;
        ExportMode mode;
        StandardExportConfig* config = nullptr;
        QString format;
        ExportPlugin* plugin = nullptr;
};

#endif // EXPORTMANAGER_H
