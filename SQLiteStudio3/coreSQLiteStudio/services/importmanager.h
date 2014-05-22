#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include <QFlags>
#include <QStringList>

class ImportPlugin;
class Db;
class CfgEntry;

class ImportManager : public QObject
{
        Q_OBJECT

    public:
        struct StandardImportConfig
        {
            /**
             * @brief Text encoding.
             *
             * Always one of QTextCodec::availableCodecs().
             * Codec is important for text-based data. For binary data it should irrelevant to the import plugin.
             */
            QString codec;

            /**
             * @brief Name of the file that the import is being done from.
             *
             * This is provided just for information to the import process,
             * but the plugin should use data stream provided to each called import method,
             * instead of opening the file from this name.
             *
             * It will be null string if importing is not performed from a file, but from somewhere else
             * (for example from a clipboard).
             */
            QString inputFileName;
        };

        enum StandardConfigFlag
        {
            CODEC             = 0x01, /**< Text encoding (see StandardImportConfig::codec). */
            FILE_NAME         = 0x02, /**< Input file (see StandardImportConfig::inputFileName). */
        };

        Q_DECLARE_FLAGS(StandardConfigFlags, StandardConfigFlag)

        ImportManager();

        QStringList getImportDataSourceTypes() const;
        ImportPlugin* getPluginForDataSourceType(const QString& dataSourceType) const;

        void configure(const QString& dataSourceType, const StandardImportConfig& config);
        void importToTable(Db* db, const QString& table);

        /**
         * @brief Available for import plugins to report validation errors on their UI forms.
         * @param configValid If the config value is valid or not.
         * @param key The config key that was validated.
         * @param errorMessage if the \p valid is false, then the \p errorMessage can carry the details of the validation result.
         *
         * Since import plugins themself are independet from QtGui, they still can provide *.ui files
         * and they can use CFG_CATEGORIES to bind with *.ui files, then they can validate values
         * stored in the CFG_CATEGORIES. In case that some value is invalid, they should call
         * this method to let the UI know, that the widget should be marked for invalid value.
         */
        void handleValidationFromPlugin(bool configValid, CfgEntry* key, const QString& errorMessage = QString());

        /**
         * @brief Available for import plugins to update UI of their options accordingly to the config values.
         * @param key The config key that the update is about.
         * @param visible The visibility for the widget.
         * @param enabled Enabled/disabled state for the widget.
         *
         * This method is here for the same reason that the handleValidationFromPlugin() is.
         */
        void updateVisibilityAndEnabled(CfgEntry* key, bool visible, bool enabled);

        static bool isAnyPluginAvailable();

    private:
        StandardImportConfig importConfig;
        ImportPlugin* plugin = nullptr;
        bool importInProgress = false;
        Db* db = nullptr;
        QString table;

    public slots:
        void interrupt();

    private slots:
        void finalizeImport(bool result);

    signals:
        void importFinished();
        void importSuccessful();
        void importFailed();
        void orderWorkerToInterrup();

        /**
         * @brief Emitted when import plugin performed its configuration validation.
         * @param valid true if plugin accepts its configuration.
         * @param key a key that cause valid/invalid state.
         * @param errorMessage if the \p valid is false, then the \p errorMessage can carry the details of the validation result.
         *
         * Slot handling this signal should update UI to reflect the configuration state.
         */
        void validationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMessage);

        /**
         * @brief Emitted then import plugin wants to update UI according to config values.
         * @param key The config key that the update is about.
         * @param visible The visibility for the widget.
         * @param enabled Enabled/disabled state for the widget.
         *
         * Slot handling this signal should update UI to reflect the state provided in parameters.
         */
        void stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled);

};

#define IMPORT_MANAGER SQLITESTUDIO->getImportManager()

Q_DECLARE_OPERATORS_FOR_FLAGS(ImportManager::StandardConfigFlags)

#endif // IMPORTMANAGER_H
