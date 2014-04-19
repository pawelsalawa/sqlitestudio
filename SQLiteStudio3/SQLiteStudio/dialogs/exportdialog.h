#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "services/exportmanager.h"
#include <QWizard>

namespace Ui {
    class ExportDialog;
}

class DbListModel;
class DbObjListModel;
class SelectableDbObjModel;
class WidgetCover;
class ConfigMapper;

class ExportDialog : public QWizard
{
        Q_OBJECT

    public:
        explicit ExportDialog(QWidget *parent = 0);
        ~ExportDialog();

        void setTableMode(Db* db, const QString& table);
        void setQueryMode(Db* db, const QString& query);
        void setDatabaseMode(Db* db);
        int nextId() const;
        bool isPluginConfigValid() const;

    protected:
        void resizeEvent(QResizeEvent* e);

    private:
        void init();
        void initModePage();
        void initTablePage();
        void initQueryPage();
        void initDbObjectsPage();
        void initFormatPage();
        void initPageOrder();
        int pageId(QWizardPage* wizardPage) const;
        void tablePageDisplayed();
        void queryPageDisplayed();
        void dbObjectsPageDisplayed();
        void formatPageDisplayed();
        ExportPlugin* getSelectedPlugin() const;
        void updatePluginOptions(ExportPlugin* plugin, int& optionsRow);
        void doExport();
        void exportDatabase(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        void exportTable(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        void exportQuery(const ExportManager::StandardExportConfig& stdConfig, const QString& format);
        ExportManager::StandardExportConfig getExportConfig() const;
        Db* getDbForExport(const QString& name);
        void notifyInternalError();

        QHash<ExportManager::ExportMode,QList<QWizardPage*>> pageOrder;

        Ui::ExportDialog *ui;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
        Db* db = nullptr;
        QString query;
        QString table;
        DbListModel* dbListModel = nullptr;
        DbObjListModel* tablesModel = nullptr;
        SelectableDbObjModel* selectableDbListModel = nullptr;
        QWidget* pluginOptionsWidget = nullptr;
        bool tablePageVisited = false;
        bool queryPageVisited = false;
        bool dbObjectsPageVisited = false;
        bool formatPageVisited = false;
        WidgetCover* widgetCover = nullptr;
        ConfigMapper* configMapper = nullptr;
        QHash<CfgEntry*,bool> pluginConfigOk;

    private slots:
        void handleValidationResultFromPlugin(bool valid, CfgEntry* key);
        void updateExportMode();
        void pageChanged(int pageId);
        void updateDbTables();
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

    public slots:
        void accept();

    signals:
        void formatPageCompleteChanged();
        void tablePageCompleteChanged();
        void queryPageCompleteChanged();
};

#endif // EXPORTDIALOG_H
