#ifndef IMPORTPLUGIN_H
#define IMPORTPLUGIN_H

#include "plugin.h"
#include "services/importmanager.h"

class QIODevice;
class CfgMain;

/**
 * @brief Provides support for particular import format.
 *
 * All import methods in this class should report any warnings, error messages, etc through the NotifyManager,
 * that is by using notifyError() and its family methods.
 */
class ImportPlugin : virtual public Plugin
{
    public:
        /**
         * @brief Pair of column name and its data type.
         */
        typedef QPair<QString,QString> ColumnDefinition;

        /**
         * @brief Used to show this plugin in the combo of data source types in the import dialog.
         * @return String representing this plugin in the import dialog.
         */
        virtual QString getDataSourceTypeName() const = 0;

        /**
         * @brief Tells which standard import options should be available on to the user.
         * @return OR-ed set of standard option enums.
         */
        virtual ImportManager::StandardConfigFlags standardOptionsToEnable() const = 0;

        /**
         * @brief Provides file name filter for file dialog.
         * @return Filter compliant with QFileDialog documentation.
         *
         * If your plugin does not return ImportManager::FILE_NAME, this method can simply return QString::null.
         * If your plugin does use input file name, then this method can (but don't have to) return file name filter
         * to match expected files when user browses for the input file.
         *
         * The filter value (if not null) is passed directly to the QFileDialog.
         */
        virtual QString getFileFilter() const = 0;

        /**
         * @brief Called before each import that user makes.
         * @param config Standard options configured by user in the import dialog.
         * @return true if everything looks fine from plugin's perspective, or false otherwise.
         *
         * In case there were some problems at this step, plugin should return false, but before that it should tell what was wrong using NotifyManager's global shortcut
         * method: notifyError().
         */
        virtual bool beforeImport(const ImportManager::StandardImportConfig& config) = 0;

        /**
         * @brief Called after import process has been finished (successfully or not).
         *
         * Implement this method to clean up any resources that the plugin has initialized before.
         */
        virtual void afterImport() = 0;

        /**
         * @brief Provides list of columns (with their datatypes) for the data to be imported.
         * @return List of columns, each column consists of column name and its data type definition.
         *
         * The ColumnDefinition is actually a QPair of two QString types. First in the pair is column name, second is column's data type,
         * as a string representation.
         *
         * Let's say your plugin wants to import data that fits into 2 columns, first of <tt>INTEGER</tt> type and the second of <tt>VARCHAR(0, 5)</tt> type.
         * You would write it like this:
         * @code
         * QList<ColumnDefinition> list;
         * list << ColumnDefinition("column1", "INTEGER");
         * list << ColumnDefinition("column2", "VARCHAR (0, 5)");
         * return list;
         * @endcode
         */
        virtual QList<ColumnDefinition> getColumns() const = 0;

        /**
         * @brief Provides next set of data from the data source.
         * @return List of values, where number of elements must be equal to number of columns returned from getColumns().
         *
         * This is essential import plugin method. It provides the data.
         * This method simply provides next row of the data for a table.
         * It will be called again and again, until it returns empty list, which will be interpreted as the end of data to import.
         */
        virtual QList<QVariant> next() = 0;

        /**
         * @brief Provides config object that holds configuration for importing.
         * @return Config object, or null if the importing with this plugin is not configurable.
         */
        virtual CfgMain* getConfig() = 0;

        /**
         * @brief Provides name of the form to use for configuration of import dialog.
         * @return Name of the form (toplevel QWidget in the ui file).
         *
         * If importing with this plugin is not configurable (i.e. getConfig() returns null),
         * then this method is not even called, so it can return anything, just to satisfy method
         * return type. In that case good idea is to always return QString::null.
         *
         * @see FormManager
         */
        virtual QString getImportConfigFormName() const = 0;

        /**
         * @brief Called when the UI expects any configuration options to be re-validated.
         *
         * When user interacts with the UI in a way that it doesn't change the config values,
         * but it still requires some options to be re-validated, this method is called.
         *
         * It should validate any configuration values defined with CFG_CATEGORY and CFG_ENTRY
         * and post the validation results by calling IMPORT_MANAGER->handleValidationFromPlugin()
         * for every validated CfgEntry.
         *
         * This is also a good idea to connect to the CfgEntry::changed() signal for entries that should be validated
         * and call this method from the slot, so any changes to the configuration values will be
         * immediately validated and reflected on the UI.
         *
         * In this method you can also call IMPORT_MANAGER->configStateUpdateFromPlugin() to adjust options UI
         * to the current config values.
         */
        virtual void validateOptions() = 0;
};

#endif // IMPORTPLUGIN_H
