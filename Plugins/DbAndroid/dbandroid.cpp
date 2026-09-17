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
#include "common/utils.h"
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

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
    QString defaultCipher = "aes256cbc";

    QMap<QString, QVariant> ciphers;
    ciphers.insert("wxSQLite3: AES 128 Bit", QVariant("aes128cbc"));
    ciphers.insert("wxSQLite3: AES 256 Bit", QVariant("aes256cbc"));
    ciphers.insert("sqleet: ChaCha20-Poly1305", QVariant("chacha20"));
    ciphers.insert("SQLCipher: AES 256 Bit", QVariant("sqlcipher"));
    ciphers.insert("System.Data.SQLite: RC4", QVariant("rc4"));
    ciphers.insert("Ascon-128 v1.2", QVariant("ascon128"));
    ciphers.insert("AEGIS", QVariant("aegis"));

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

    DbPluginOption optPass;
    optPass.type = DbPluginOption::PASSWORD;
    optPass.key = PASSWORD_OPT;
    optPass.label = tr("Password (key)");
    optPass.toolTip = tr("Leave empty to create or connect to decrypted database.");
    optPass.placeholderText = tr("Encryption password");
    options << optPass;

    DbPluginOption optCipher;
    optCipher.type = DbPluginOption::CHOICE;
    optCipher.key = CIPHER_OPT;
    optCipher.label = tr("Cipher");
    optCipher.toolTip = tr("Cipher determines encryption algorithm used to encrypt the database.");
    optCipher.choiceDataValues = ciphers;
    optCipher.defaultValue = defaultCipher;
    options << optCipher;

    DbPluginOption optPragmas;
    optPragmas.type = DbPluginOption::SQL;
    optPragmas.key = PRAGMAS_OPT;
    optPragmas.defaultValue = "-- Optional PRAGMA statements for cipher configuration";
    optPragmas.label = tr("Cipher configuration (optional)");
    optPragmas.toolTip = tr("PRAGMA statements to customize cipher configuration, such as KDF iterations, legacy mode, etc.\nThey will be executed upon each opening of the database.");
    options << optPragmas;

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

    connect(ICONMANAGER, SIGNAL(rescannedFor(QString)), this, SLOT(createAarAction(QString)));

    connectionFactory = new DbAndroidConnectionFactory(this);

    adbManager = new AdbManager(this);
    connect(adbManager, SIGNAL(deviceListChanged(QStringList)), this, SLOT(deviceListChanged()));

    if (adbManager->testCurrentAdb())
    {
        qDebug() << "Using ADB binary:" << cfg.DbAndroid.AdbPath.get();
        adbValid = true;
        adbManager->getDevices(true);
        showAarMessage();
    }
    else
        runInThread([=, this]{ initAdb(); });

    return true;
}

void DbAndroid::deinit()
{
    if (aarAction)
        MAINWINDOW->getToolsMenu()->removeAction(aarAction);

    safe_delete(aarAction);
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

    showAarMessage();
    DBLIST->rescanInvalidDatabasesForPlugin(this);
}

void DbAndroid::showAarMessage()
{
    if (!cfg.DbAndroid.AarDownloadNotified.get())
    {
        notifyInfo(tr("You can grab Android connector AAR file from Tools menu. It's required for 2 of 3 connections supported by the Android plugin. "
                      "For more details read plugin's documentation on <a href=\"%1\">Letos's wiki page.</a>").arg(PLUGIN_MANUAL_URL));
        cfg.DbAndroid.AarDownloadNotified.set(true);
    }
}

void DbAndroid::createAarAction()
{
    QIcon i = ICONMANAGER->getIcon("android");
    aarAction = MAINWINDOW->getToolsMenu()->addAction(i, tr("Get Android connector AAR file"));
    connect(aarAction, SIGNAL(triggered()), this, SLOT(getAar()));
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

            QMessageBox box(QMessageBox::Warning, tr("Invalid ADB"),
                            tr("The selected ADB is incorrect."), QMessageBox::NoButton, MAINWINDOW);
            box.setInformativeText(tr("Would you like to select another one, or leave it unconfigured?"));
            box.setDefaultButton(box.addButton(tr("Select another ADB"), QMessageBox::ResetRole));
            QAbstractButton* rejectButton = box.addButton(tr("Leave unconfigured"), QMessageBox::RejectRole);
            box.exec();

            if (box.clickedButton() == rejectButton)
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

void DbAndroid::getAar()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, tr("Save AAR file"));
    if (path.isEmpty())
        return;

    QString newPath = path + "/letosremote.aar";
    QFile aarFile(":/dbandroid/letosremote.aar");
    if (aarFile.copy(newPath)) {
        notifyInfo("Android connector AAR file saved at " + newPath);
    } else {
        notifyError("Unable to save android connector AAR file at " + newPath);
    }
}

void DbAndroid::createAarAction(const QString& pluginName)
{
    if (pluginName != "" && pluginName != getName())
        return;

    createAarAction();
}

AdbManager* DbAndroid::getAdbManager() const
{
    return adbManager;
}
