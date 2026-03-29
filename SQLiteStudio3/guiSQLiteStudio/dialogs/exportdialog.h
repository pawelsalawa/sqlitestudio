#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "guiSQLiteStudio_global.h"
#include "services/exportmanager.h"
#include <QWizard>
#include <QHash>

namespace Ui {
    class ExportDialog;
}

class DbListModel;
class DbObjListModel;
class SelectableDbObjModel;
class WidgetCover;
class ConfigMapper;
class QComboBox;

class GUI_API_EXPORT ExportDialog : public QWizard
{
        Q_OBJECT

    public:
        explicit ExportDialog(QWidget *parent = 0);
        ~ExportDialog();

        void setTableMode(Db* db, const QString& table);
        void setViewMode(Db* db, const QString& view);
        void setQueryMode(Db* db, const QString& query);
        void setDatabaseMode(Db* db, bool syncSelectedItems);
        void setPreselectedDb(Db* db, bool syncSelectedItems);
        int nextId() const;
        bool isPluginConfigValid() const;

    private:
        void init();
        void initModePage();
        void initTablePage();
        void initViewPage();
        void initQueryPage();
        void initDbObjectsPage();
        void initFormatPage();
        void initPageOrder();
        int pageId(QWizardPage* wizardPage) const;
        void tablePageDisplayed();
        void viewPageDisplayed();
        void queryPageDisplayed();
        void dbObjectsPageDisplayed();
        void formatPageDisplayed();
        ExportPlugin* getSelectedPlugin() const;
        void updatePluginOptions(ExportPlugin* plugin, int& optionsRow);
        void storeStdConfig(const ExportManager::StandardExportConfig& stdConfig);
        void readStdConfigForFirstPage();
        void readStdConfigForLastPage();
        void doExport();
        void exportDatabase(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        void exportTable(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        void exportView(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        void exportQuery(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        ExportManager::StandardExportConfig getExportConfig() const;
        Db* getDbForExport(const QString& name);
        void notifyInternalError();
        QModelIndex setupNewDbObjTreeRoot(const QModelIndex& root);
        QComboBox* getDbComboForCurrentPage() const;

        QHash<ExportManager::ExportMode,QList<QWizardPage*>> pageOrder;

        Ui::ExportDialog *ui = nullptr;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
        Db* db = nullptr;
        QString query;
        QString table;
        QString view;
        DbListModel* dbListModel = nullptr;
        DbObjListModel* tablesModel = nullptr;
        DbObjListModel* viewsModel = nullptr;
        SelectableDbObjModel* selectableDbListModel = nullptr;
        QWidget* pluginOptionsWidget = nullptr;
        bool tablePageVisited = false;
        bool viewPageVisited = false;
        bool queryPageVisited = false;
        bool dbObjectsPageVisited = false;
        bool formatPageVisited = false;
        WidgetCover* widgetCover = nullptr;
        ConfigMapper* configMapper = nullptr;
        QHash<CfgEntry*,bool> pluginConfigOk;
        ExportPlugin* currentPlugin = nullptr;
        bool syncDbItemsInitially = false;

    private slots:
        void handleValidationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMsg);
        void stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled);
        void updateExportMode();
        void pageChanged(int pageId);
        void updateDbTables();
        void updateDbViews();
        void browseForExportFile();
        void pluginSelected();
        void updateExportOutputOptions();
        void updateQueryEditDb();
        void updateOptions();
        void updateDbObjTree();
        void dbObjectsSelectAll();
        void dbObjectsDeselectAll();
        void hideCoverWidget();
        void storeInClipboard(const QByteArray& bytes, const QString& mimeType);
        void storeInClipboard(const QString& str);
        void success();
        void updateValidation();
        void prevDb();
        void nextDb();

    public slots:
        void accept();
        int exec();

    signals:
        void formatPageCompleteChanged();
        void tablePageCompleteChanged();
        void viewPageCompleteChanged();
        void queryPageCompleteChanged();
};

#endif // EXPORTDIALOG_H
