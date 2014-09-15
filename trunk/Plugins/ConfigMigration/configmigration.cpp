#include "configmigration.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

ConfigMigration::ConfigMigration()
{
}

bool ConfigMigration::init()
{
    QString oldCfg = findOldConfig();
    if (!oldCfg.isNull())
        notifyInfo(tr("A configuration from old SQLiteStudio 2.x.x has been detected. "
                      "Would you like to migrate old settings into the current version? "
                      "<a href=\"%1\">Click here to do that</a>.").arg("migrateOldConfig"));

    return true;
}

QString ConfigMigration::findOldConfig()
{
    QString output;
    QString dirPath;

    // Portable path 1 check
    dirPath = QDir::currentPath() + "/sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    // Portable path 2 check
    dirPath = QCoreApplication::applicationDirPath() + "/sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    // Portable path 3 check
    dirPath = QCoreApplication::applicationDirPath() + "/../sqlitestudio-cfg";
    if (checkOldDir(dirPath, output))
        return output;

    if (getDistributionType() == DistributionType::OSX_BOUNDLE)
    {
        // Portable path 4 check
        dirPath = QCoreApplication::applicationDirPath() + "/../../sqlitestudio-cfg";
        if (checkOldDir(dirPath, output))
            return output;

        // Portable path 5 check
        dirPath = QCoreApplication::applicationDirPath() + "/../../../sqlitestudio-cfg";
        if (checkOldDir(dirPath, output))
            return output;
    }

    // Global path check
#ifdef Q_OS_WIN
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based)
        dirPath = SQLITESTUDIO->getEnv("APPDATA")+"/sqlitestudio";
    else
        dirPath = SQLITESTUDIO->getEnv("HOME")+"/sqlitestudio";
#else
    dirPath = SQLITESTUDIO->getEnv("HOME")+"/.sqlitestudio";
#endif

    if (checkOldDir(dirPath, output))
        return output;

    return QString();
}

bool ConfigMigration::checkOldDir(const QString &dir, QString &output)
{
    QFileInfo fi(dir + "/settings");
    if (fi.exists() && fi.isReadable())
    {
        output = fi.absoluteFilePath();
        return true;
    }

    return false;
}
