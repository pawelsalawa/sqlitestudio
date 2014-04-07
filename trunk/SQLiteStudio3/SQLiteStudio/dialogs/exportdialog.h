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

    private slots:
        void updateExportMode();
        void pageChanged(int pageId);
        void updateDbTables();
        void browseForExportFile();
        void pluginSelected();
        void updateDbObjTree();
        void dbObjectsSelectAll();
        void dbObjectsDeselectAll();
};

#endif // EXPORTDIALOG_H
