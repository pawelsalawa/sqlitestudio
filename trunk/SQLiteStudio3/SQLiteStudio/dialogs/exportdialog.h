#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "services/exportmanager.h"
#include <QWizard>

namespace Ui {
    class ExportDialog;
}

class DbListModel;
class DbObjListModel;

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
        void initFormatPage();
        void initPageOrder();
        int pageId(QWizardPage* wizardPage) const;
        void tablePageDisplayed();
        void queryPageDisplayed();
        void dbObjectsPageDisplayed();
        void formatPageDisplayed();

        QHash<ExportManager::ExportMode,QList<QWizardPage*>> pageOrder;

        Ui::ExportDialog *ui;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
        Db* db = nullptr;
        QString query;
        QString table;
        DbListModel* dbListModel = nullptr;
        DbObjListModel* dbObjListModel = nullptr;

    private slots:
        void updateExportMode();
        void pageChanged(int pageId);
        void updateDbTables();
        void browseForExportFile();

    public slots:
        int exec();
};

#endif // EXPORTDIALOG_H
