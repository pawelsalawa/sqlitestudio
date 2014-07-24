#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QWizard>

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

class ImportDialog : public QWizard
{
        Q_OBJECT

    public:
        explicit ImportDialog(QWidget *parent = 0);
        ~ImportDialog();

        void setDbAndTable(Db* db, const QString& table);
        void setDb(Db* db);

    protected:
        void showEvent(QShowEvent* e);

    private:
        void init();
        void initTablePage();
        void initDataSourcePage();
        void removeOldOptions();
        void updateStandardOptions();
        void updatePluginOptions(int& rows);
        bool isPluginConfigValid() const;

        Ui::ImportDialog *ui;
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
