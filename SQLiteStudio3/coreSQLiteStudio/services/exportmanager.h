#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "db/sqlquery.h"
#include "db/db.h"
#include "pluginservicebase.h"
#include "sqlitestudio.h"
#include <QObject>

class ExportPlugin;
class ExportWorker;
class QBuffer;
class CfgEntry;

/**
 * @brief Provides database exporting capabilities.
 *
 * ExportManager is not thread-safe. Use it from single thread.
 */
class API_EXPORT ExportManager : public PluginServiceBase
{
        Q_OBJECT
    public:
        enum ExportMode
        {
            UNDEFINED     = 0x00,
            CLIPBOARD     = 0x01,
            DATABASE      = 0x02,
            TABLE         = 0x04,
            QUERY_RESULTS = 0x08,
            FILE          = 0x10
        };

        Q_DECLARE_FLAGS(ExportModes, ExportMode)

        /**
         * @brief Flags for requesting additional information for exporting by plugins.
         *
         * Each plugin implementation might ask ExportWorker to provide additional information for exporting.
         * Such information is usually expensive operation (an additional database query to execute), therefore
         * they are not enabled by default for all plugins. Each plugin has to ask for them individually
         * by returning this enum values from ExportPlugin::getProviderFlags().
         *
         * For each enum value returned from the ExportPlugin::getProviderFlags(), a single QHash entry will be prepared
         * and that hash will be then passed to one of ExportPlugin::beforeExportQueryResults(), ExportPlugin::exportTable(),
         * or ExportPlugin::exportVirtualTable(). If no flags were returned from ExportPlugin::getProviderFlags(), then
         * empty hash will be passed to those methods.
         *
         * Each entry in the QHash has a key equal to one of values from this enum. Values from the hash are of QVariant type
         * and therefore they need to be casted (by QVariant means) into desired type. For each enum value its description
         * will tell you what actually is stored in the QVariant, so you can extract the information.
         */
        enum ExportProviderFlag
        {
            NONE          = 0x00, /**< This is a default. Nothing will be stored in the hash. */
            DATA_LENGTHS  = 0x01, /**<
                                    * Will provide maximum number of characters or bytes (depending on column type)
                                    * for each exported table or qurey result column. It will be a <tt>QList&lt;int&gt;</tt>.
                                    */
            ROW_COUNT     = 0x02  /**<
                                    * Will provide total number of rows that will be exported for the table or query results.
                                    * It will be an integer value.
                                    */
        };

        Q_DECLARE_FLAGS(ExportProviderFlags, ExportProviderFlag)

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
            SqlQueryPtr data;
            QHash<ExportManager::ExportProviderFlag,QVariant> providerData;
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

            /**
             * @brief When exporting only a table, this indicates if indexes related to that table should also be exported.
             *
             * Default is true.
             */
            bool exportTableIndexes = true;

            /**
             * @brief When exporting only a table, this indicates if triggers related to that table should also be exported.
             *
             * Default is true.
             */
            bool exportTableTriggers = true;
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

        QStringList getAvailableFormats(ExportMode exportMode = UNDEFINED) const;
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

        static bool isAnyPluginAvailable();

    private:
        void invalidFormat(const QString& format);
        bool checkInitialConditions();
        QIODevice* getOutputStream();
        ExportWorker* prepareExport();
        void handleClipboardExport();

        bool exportInProgress = false;
        ExportMode mode;
        StandardExportConfig* config = nullptr;
        QString format;
        ExportPlugin* plugin = nullptr;
        QBuffer* bufferForClipboard = nullptr;

    public slots:
        void interrupt();

    private slots:
        void finalizeExport(bool result, QIODevice* output);

    signals:
        void exportFinished();
        void exportSuccessful();
        void exportFailed();
        void finishedStep(int step);
        void storeInClipboard(const QString& str);
        void storeInClipboard(const QByteArray& bytes, const QString& mimeType);
        void orderWorkerToInterrupt();
};

#define EXPORT_MANAGER SQLITESTUDIO->getExportManager()

Q_DECLARE_OPERATORS_FOR_FLAGS(ExportManager::StandardConfigFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExportManager::ExportModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExportManager::ExportProviderFlags)

#endif // EXPORTMANAGER_H
