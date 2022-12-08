#include "services/config.h"
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

CFG_DEFINE(Core)

static const QString DB_FILE_NAME = QStringLiteral("settings3");
static QString MASTER_CONFIG_FILE = QString();
Config::AskUserForConfigDirFunc Config::askUserForConfigDirFunc;
QSettings* globalSettingsInstance = nullptr;

Config::~Config()
{
}

void Config::setMasterConfigFile(const QString &path)
{
    MASTER_CONFIG_FILE = path;
}

QString Config::getMasterConfigFile()
{
    return MASTER_CONFIG_FILE;
}

void Config::setAskUserForConfigDirFunc(const AskUserForConfigDirFunc& value)
{
    askUserForConfigDirFunc = value;
}

QString Config::getPortableConfigPath()
{
    QStringList paths = QStringList({"./sqlitestudio-cfg", qApp->applicationDirPath() + "/sqlitestudio-cfg"});
    QSet<QString> pathSet;
    QDir dir;
    for (const QString& path : paths)
    {
        dir = QDir(path);
        pathSet << dir.absolutePath();
    }

    QString potentialPath;
    QFileInfo file;
    for (const QString& path : pathSet)
    {
        dir = QDir(path);
        file = QFileInfo(dir.absolutePath());
        if (!file.exists())
        {
            if (potentialPath.isNull())
                potentialPath = dir.absolutePath();

            continue;
        }

        if (!file.isDir() || !file.isReadable() || !file.isWritable())
            continue;

        for (QFileInfo& entryFile : dir.entryInfoList())
        {
            if (!entryFile.isReadable() || !entryFile.isWritable())
                continue;
        }

        return dir.absolutePath();
    }

    return potentialPath;
}

QSettings* Config::getSettings()
{
    if (globalSettingsInstance == nullptr)
    {
        QString portableConfigPath = Config::getPortableConfigPath();
        QFileInfo portableConfigInfo(portableConfigPath);
        if (portableConfigInfo.exists() && portableConfigInfo.isDir() && portableConfigInfo.isReadable())
            globalSettingsInstance = new QSettings(portableConfigPath + "/settings.ini", QSettings::IniFormat);
        else
            globalSettingsInstance = new QSettings();
    }

    return globalSettingsInstance;
}
