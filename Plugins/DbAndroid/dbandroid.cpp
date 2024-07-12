#include "adbmanager.h"
#include "dbandroid.h"
#include "dbandroidinstance.h"
#include "dbandroidpathdialog.h"
#include "mainwindow.h"
#include "services/notifymanager.h"
#include "uiconfig.h"
#include "statusfield.h"
#include "services/dbmanager.h"
#include "dbandroidconnectionfactory.h"
#include "iconmanager.h"
#include <QUrl>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>

DbAndroid::DbAndroid()
{
}

QString DbAndroid::getLabel() const
{
    return "Android SQLite";
}

bool DbAndroid::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbAndroidInstance*>(db));
}

Db* DbAndroid::getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage)
{
    DbAndroidUrl url(path);
    if (!url.isValid())
    {
        if (errorMessage)
            *errorMessage = tr("Invalid or incomplete Android Database URL.");

        return nullptr;
    }

    DbAndroidInstance* db = new DbAndroidInstance(this, name, path, options);
    return db;
}

QList<DbPluginOption> DbAndroid::getOptionsList() const
{
    QList<DbPluginOption> options;

    DbPluginOption customBrowseOpt;
    customBrowseOpt.type = DbPluginOption::Type::CUSTOM_PATH_BROWSE;
    customBrowseOpt.label = tr("Android database URL");
    customBrowseOpt.toolTip = tr("Select Android database");
    customBrowseOpt.customBrowseHandler = [this](const QString& initialPath) -> QString
    {
        DbAndroidPathDialog dialog(this, MAINWINDOW);
        dialog.setUrl(initialPath);
        if (!dialog.exec())
            return QString();

        return dialog.getUrl().toUrlString();
    };
    options << customBrowseOpt;

    return options;
}

QString DbAndroid::generateDbName(const QVariant& baseValue)
{
    // android://drgh:port/dbName
    QUrl url(baseValue.toString());
    if (!url.isValid())
        return baseValue.toString();

    return url.fileName();
}

bool DbAndroid::init()
{
    SQLS_INIT_RESOURCE(dbandroid);

    qRegisterMetaType<QList<AdbManager::Device>>("QList<Device>");

    connect(this, SIGNAL(adbReady(bool)), this, SLOT(handleValidAdb(bool)));
    connect(this, SIGNAL(invalidAdb()), this, SLOT(handleInvalidAdb()));
    connect(MAINWINDOW->getStatusField(), SIGNAL(linkActivated(QString)), this, SLOT(statusFieldLinkClicked(QString)));

    connect(ICONMANAGER, SIGNAL(rescannedFor(QString)), this, SLOT(createJarAction(QString)));

    connectionFactory = new DbAndroidConnectionFactory(this);

    adbManager = new AdbManager(this);
    connect(adbManager, SIGNAL(deviceListChanged(QStringList)), this, SLOT(deviceListChanged()));

    if (adbManager->testCurrentAdb())
    {
        qDebug() << "Using ADB binary:" << cfg.DbAndroid.AdbPath.get();
        adbValid = true;
        adbManager->getDevices(true);
        showJarMessage();
    }
    else
    {
#if QT_VERSION >= 0x060000
        QtConcurrent::run(&DbAndroid::initAdb, this);
#else
        QtConcurrent::run(this, &DbAndroid::initAdb);
#endif
    }
    return true;
}

void DbAndroid::deinit()
{
    if (jarAction)
        MAINWINDOW->getToolsMenu()->removeAction(jarAction);

    safe_delete(jarAction);
    safe_delete(connectionFactory);
    safe_delete(adbManager);
    SQLS_CLEANUP_RESOURCE(dbandroid);
}

QString DbAndroid::getCurrentAdb()
{
    return cfg.DbAndroid.AdbPath.get();
}

void DbAndroid::initAdb()
{
    QString adbPath = adbManager->findAdb();
    if (!adbPath.isEmpty())
    {
        cfg.DbAndroid.AdbPath.set(adbPath);
        qDebug() << "Found ADB binary:" << cfg.DbAndroid.AdbPath.get();
        emit adbReady(true);
        return;
    }

    emit invalidAdb();
}

QString DbAndroid::askForAdbPath()
{
#if defined(Q_OS_UNIX)
        QString adbAppName = "adb";
#elif defined(Q_OS_WIN32)
        QString adbAppName = "adb.exe";
#else
        qCritical() << "Unsupported OS for DbAndroid.";
        return QString();
#endif
    QString file = QFileDialog::getOpenFileName(MAINWINDOW, tr("Select ADB"), getFileDialogInitPath(), QString("Android Debug Bridge (%1)").arg(adbAppName));
    if (file.isEmpty())
        return file;

    setFileDialogInitPathByFile(file);
    return file;
}

bool DbAndroid::isAdbValid() const
{
    return adbValid;
}

DbAndroidConnectionFactory*DbAndroid::getConnectionFactory() const
{
    return connectionFactory;
}

void DbAndroid::handleValidAdb(bool showMessage)
{
    adbValid = true;
    if (showMessage)
        notifyInfo(tr("Using Android Debug Bridge: %1").arg(cfg.DbAndroid.AdbPath.get()));

    showJarMessage();
    DBLIST->rescanInvalidDatabasesForPlugin(this);
}

void DbAndroid::showJarMessage()
{
    if (!cfg.DbAndroid.JarDownloadNotified.get())
    {
        notifyInfo(tr("You can grab Android connector JAR file from Tools menu. It's required for 2 of 3 connections supported by the Android plugin. "
                      "For more details read plugin's documentation on <a href=\"%1\">SQLiteStudio's wiki page.</a>").arg(PLUGIN_MANUAL_URL));
        cfg.DbAndroid.JarDownloadNotified.set(true);
    }
}

void DbAndroid::handleInvalidAdb()
{
    notifyError(tr("Could not find Android Debug Bridge application. <a href=\"%1\">Click here</a> to point out the location of the ADB application, "
                   "otherwise the %2 plugin will not support USB cable connections, only the network connection.").arg(SELECT_ADB_URL, getLabel()));
}

void DbAndroid::statusFieldLinkClicked(const QString& link)
{
    if (link == SELECT_ADB_URL)
    {
        QString file = askForAdbPath();
        while (!file.isEmpty())
        {
            if (adbManager->testAdb(file))
            {
                cfg.DbAndroid.AdbPath.set(file);
                emit adbReady(true);
                return;
            }

            int res = QMessageBox::warning(MAINWINDOW, tr("Invalid ADB"), tr("The selected ADB is incorrect.\n"
                                                                             "Would you like to select another one, or leave it unconfigured?"),
                                           tr("Select another ADB"), tr("Leave unconfigured"));

            if (res == 1)
                return;

            file = askForAdbPath();
        }
    }
    else if (link == PLUGIN_MANUAL_URL)
    {
        QDesktopServices::openUrl(QUrl(PLUGIN_MANUAL_URL));
    }
}

void DbAndroid::deviceListChanged()
{
    DBLIST->rescanInvalidDatabasesForPlugin(this);
}

void DbAndroid::getJar()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, tr("Save JAR file"));
    if (path.isEmpty())
        return;

    QString newPath = path + "/SQLiteStudioRemote.jar";
    QFile jarFile(":/dbandroid/SQLiteStudioRemote.jar");
    if (jarFile.copy(newPath)) {
        notifyInfo("Android connector JAR file saved at " + newPath);
    } else {
        notifyError("Unable to save android connector JAR file at " + newPath);
    }
}

void DbAndroid::createJarAction(const QString& pluginName)
{
    if (pluginName != "" && pluginName != getName())
        return;

    QIcon* i = ICONMANAGER->getIcon("android");
    jarAction = MAINWINDOW->getToolsMenu()->addAction(*(i), tr("Get Android connector JAR file"));
    connect(jarAction, SIGNAL(triggered()), this, SLOT(getJar()));
}

AdbManager* DbAndroid::getAdbManager() const
{
    return adbManager;
}
