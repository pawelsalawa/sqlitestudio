#ifndef DBANDROIDPATHDIALOG_H
#define DBANDROIDPATHDIALOG_H

#include "dbandroidurl.h"
#include <QDialog>

namespace Ui {
    class DbAndroidPathDialog;
}

class DbAndroid;
class QTimer;
class WidgetCover;
class DbAndroidInstance;

class DbAndroidPathDialog : public QDialog
{
        Q_OBJECT

    public:
        DbAndroidPathDialog(const DbAndroid* plugin, QWidget *parent = 0);
        ~DbAndroidPathDialog();
        void setUrl(const QString& url);
        void setUrl(const DbAndroidUrl& url);
        const DbAndroidUrl& getUrl() const;

    private:
        void init();
        void updateUrl();
        void loadUrl();
        void asyncDbUpdate(const QString& connectionUrl, DbAndroidMode enforcedMode);
        void asyncAppUpdate(const QString& connectionUrl, DbAndroidMode enforcedMode);
        void refreshDevices();
        DbAndroidMode getSelectedMode() const;
        void setDbListUpdatesEnabled(bool enabled);

        const DbAndroid* plugin = nullptr;
        DbAndroidUrl dbUrl;
        Ui::DbAndroidPathDialog *ui;
        QTimer* dbListUpdateTimer = nullptr;
        QTimer* appListUpdateTimer = nullptr;
        WidgetCover* dbListCover = nullptr;
        WidgetCover* appListCover = nullptr;
        bool updatingDbList = false;
        bool updatingAppList = false;
        bool suspendDbListUpdates = false;
        bool suspendAppListUpdates = false;
        QStringList fullAppList;

    private slots:
        void scheduleDbListUpdate();
        void scheduleAppListUpdate();
        void updateState();
        void refreshDbList();
        void refreshAppList();
        void updateDeviceList();
        void updateValidations();
        void handleUpdateDbList(const QStringList& dbList);
        void handleUpdateAppList(const QStringList& apps);
        void handleFinishedAsyncDbListUpdate(bool appOkay);
        void handleFinishedAsyncAppListUpdate();
        void handleDbCreationUpdate(bool canCreateDatabases);
        void createNewDatabase();
        void deleteSelectedDatabase();
        void modeChanged(bool checked);
        void applyAppFilter(const QString& value);

    public slots:
        void accept();

    signals:
        void callForValidations();
        void callForDbCreationUpdate(bool canCreateDatabases);
        void asyncDbListUpdatingFinished(bool appOkay);
        void asyncAppListUpdatingFinished();
        void callForDbListUpdate(const QStringList& newList);
        void callForAppListUpdate(const QStringList& newList);
};

#endif // DBANDROIDPATHDIALOG_H
