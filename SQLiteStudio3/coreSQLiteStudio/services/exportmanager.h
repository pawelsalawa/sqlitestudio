#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "db/sqlresults.h"
#include "db/db.h"
#include <QObject>

class ExportPlugin;
class QueryExecutor;
class ExportWorker;
class QBuffer;
class CfgEntry;

/**
 * @brief Provides database exporting capabilities.
 *
 * ExportManager is not thread-safe. Use it from single thread.
 */
class API_EXPORT ExportManager : public QObject
{
        Q_OBJECT
    public:
        enum ExportMode
        {
            UNDEFINED     = 0x00,
            CLIPBOARD     = 0x01,
            DATABASE      = 0x02,
            TABLE         = 0x04,
            QUERY_RESULTS = 0x08
        };

        Q_DECLARE_FLAGS(ExportModes, ExportMode)

        struct ExportObject
        {
            enum Type
            {
                TABLE,
                INDEX,
                TRIGGER,
                VIEW
            };

            Type type;
            QString database; // TODO fill when dbnames are fully supported
            QString name;
            QString ddl;
            SqlResultsPtr data;
        };

        typedef QSharedPointer<ExportObject> ExportObjectPtr;

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
        };

        /**
         * @brief Standard export configuration options to be displayed on UI.
         *
         * Each of enum represents single property of StandardExportConfig which will be
         * available on UI to configure.
         */
        enum StandardConfigFlag
        {
            CODEC             = 0x01, /**< Text encoding (see StandardExportConfig::codec). */
        };

        Q_DECLARE_FLAGS(StandardConfigFlags, StandardConfigFlag)


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
        void configure(const QString& format, StandardExportConfig* config);

        /**
         * @brief Configures export service for export.
         * @param format Format to be used in upcoming export.
         * @param config Standard configuration options to be used in upcoming export.
         *
         * Same as method above, except it makes its own copy of the config object.
         */
        void configure(const QString& format, const StandardExportConfig& config);
        bool isExportInProgress() const;
        void exportQueryResults(Db* db, const QString& query);
        void exportTable(Db* db, const QString& database, const QString& table);
        void exportDatabase(Db* db, const QStringList& objectListToExport);

        void handleValidationFromPlugin(bool configValid, CfgEntry* key);

        static bool isAnyPluginAvailable();

    private:
        void invalidFormat(const QString& format);
        bool checkInitialConditions();
        QIODevice* getOutputStream();
        ExportWorker* prepareExport();
        void handleClipboardExport();

        bool exportInProgress = false;
        QueryExecutor* executor;
        ExportMode mode;
        StandardExportConfig* config = nullptr;
        QString format;
        ExportPlugin* plugin = nullptr;
        QBuffer* bufferForClipboard = nullptr;

    private slots:
        void finalizeExport(bool result, QIODevice* output);

    signals:
        void exportFinished();
        void exportSuccessful();
        void exportFailed();
        void storeInClipboard(const QString& str);
        void storeInClipboard(const QByteArray& bytes, const QString& mimeType);

        /**
         * @brief Emitted when export plugin performed its configuration validation.
         * @param valid true if plugin accepts its configuration.
         * @param key a key that cause valid/invalid state.
         *
         * Slot handling this signal should update UI to reflect the configuration state.
         */
        void validationResultFromPlugin(bool valid, CfgEntry* key);
};

#define EXPORT_MANAGER SQLITESTUDIO->getExportManager()

Q_DECLARE_OPERATORS_FOR_FLAGS(ExportManager::StandardConfigFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExportManager::ExportModes)

#endif // EXPORTMANAGER_H
