#ifndef DBANDROID_H
#define DBANDROID_H

#include "dbandroid_global.h"
#include "plugins/dbplugin.h"
#include "plugins/genericplugin.h"
#include "config_builder.h"

class AdbManager;
class DbAndroidConnectionFactory;
class QAction;

CFG_CATEGORIES(DbAndroidConfig,
    CFG_CATEGORY(DbAndroid,
        CFG_ENTRY(QString, AdbPath, QString())
        CFG_ENTRY(bool,    JarDownloadNotified, false)
    )
)

class DBANDROIDSHARED_EXPORT DbAndroid : public GenericPlugin, public DbPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("dbandroid.json")

    public:
        DbAndroid();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QList<DbPluginOption> getOptionsList() const;
        QString generateDbName(const QVariant& baseValue);
        bool init();
        void deinit();
        QString getCurrentAdb();
        AdbManager* getAdbManager() const;
        bool isAdbValid() const;
        DbAndroidConnectionFactory* getConnectionFactory() const;

        static_char* PASSWORD_OPT = "remote_access_password";

    private:
        void initAdb();
        QString askForAdbPath();
        void showJarMessage();
        void createJarAction();

        AdbManager* adbManager = nullptr;
        DbAndroidConnectionFactory* connectionFactory = nullptr;
        bool adbValid = false;
        QAction* jarAction = nullptr;

        static_char* PLUGIN_MANUAL_URL = "https://github.com/pawelsalawa/sqlitestudio/wiki/DbAndroid";
        static_char* SELECT_ADB_URL = "select_adb://";

        CFG_LOCAL_PERSISTABLE(DbAndroidConfig, cfg)

    private slots:
        void handleValidAdb(bool showMessage);
        void handleInvalidAdb();
        void statusFieldLinkClicked(const QString& link);
        void deviceListChanged();
        void getJar();
        void createJarAction(const QString& pluginName);

    signals:
        void adbReady(bool showMessage);
        void invalidAdb();
};

#endif // DBANDROID_H
