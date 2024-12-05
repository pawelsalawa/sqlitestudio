#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "guiSQLiteStudio_global.h"
#include "services/importmanager.h"
#include <QWizard>
#include <QHash>

namespace Ui {
    class ImportDialog;
}

class DbListModel;
class DbObjListModel;
class ImportPlugin;
class ConfigMapper;
class CfgEntry;
class WidgetCover;
class Db;

class GUI_API_EXPORT ImportDialog : public QWizard
{
        Q_OBJECT

    public:
        explicit ImportDialog(QWidget *parent = 0);
        ~ImportDialog();

        void setDbAndTable(Db* db, const QString& table);
        void setDb(Db* db);

    protected:
        void showEvent(QShowEvent* e);
        void keyPressEvent(QKeyEvent* e);

    private:
        void init();
        void initTablePage();
        void initDataSourcePage();
        void removeOldOptions();
        void updateStandardOptions();
        void updatePluginOptions(int& rows);
        bool isPluginConfigValid() const;
        void storeStdConfig(ImportManager::StandardImportConfig& stdConfig);
        void readStdConfig();

        Ui::ImportDialog *ui = nullptr;
        DbListModel* dbListModel = nullptr;
        DbObjListModel* tablesModel = nullptr;
        ConfigMapper* configMapper = nullptr;
        QWidget* pluginOptionsWidget = nullptr;
        ImportPlugin* currentPlugin = nullptr;
        QHash<CfgEntry*,bool> pluginConfigOk;
        WidgetCover* widgetCover = nullptr;

    private slots:
        void handleValidationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMsg);
        void stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled);
        void refreshTables();
        void pluginSelected();
        void updateValidation();
        void pageChanged();
        void browseForInputFile();
        void success();
        void hideCoverWidget();

    public slots:
        void accept();

    signals:
        void dsPageCompleteChanged();
};

#endif // IMPORTDIALOG_H
