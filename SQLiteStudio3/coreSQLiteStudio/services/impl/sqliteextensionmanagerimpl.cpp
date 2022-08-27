#include "sqliteextensionmanagerimpl.h"
#include "services/dbmanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

SqliteExtensionManagerImpl::SqliteExtensionManagerImpl()
{
    init();
}

void SqliteExtensionManagerImpl::setExtensions(const QList<SqliteExtensionManager::ExtensionPtr>& newExtensions)
{
    extensions = newExtensions;
    storeInConfig();
    emit extensionListChanged();
}

QList<SqliteExtensionManager::ExtensionPtr> SqliteExtensionManagerImpl::getAllExtensions() const
{
    return extensions;
}

QList<SqliteExtensionManager::ExtensionPtr> SqliteExtensionManagerImpl::getExtensionForDatabase(const QString& dbName) const
{
    QList<ExtensionPtr> results;
    for (const ExtensionPtr& ext : extensions)
    {
        if (ext->allDatabases || ext->databases.contains(dbName, Qt::CaseInsensitive))
            results << ext;
    }
    return results;
}

QStringList SqliteExtensionManagerImpl::getExtensionDirs() const
{
    return extensionDirs;
}

void SqliteExtensionManagerImpl::init()
{
    loadFromConfig();
    scanExtensionDirs();
}

void SqliteExtensionManagerImpl::scanExtensionDirs()
{
    extensionDirs += qApp->applicationDirPath() + "/extensions";
    extensionDirs += qApp->applicationDirPath() + "/ext";
    extensionDirs += QDir(CFG->getConfigDir()).absoluteFilePath("ext");
    extensionDirs += QDir(CFG->getConfigDir()).absoluteFilePath("extensions");
#ifdef Q_OS_MACX
    extensionDirs += QDir(QCoreApplication::applicationDirPath()+"/../extensions").absolutePath();
#endif

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_SQLITE_EXTENSIONS");
    if (!envDirs.isNull())
        extensionDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef SQLITE_EXTENSIONS_DIR
    extensionDirs += STRINGIFY(SQLITE_EXTENSIONS_DIR);
#endif

    for (QString& extDirPath : extensionDirs)
    {
        QDir extDir(extDirPath);
        for (QString& fileName : extDir.entryList(sharedLibFileFilters(), QDir::Files))
        {
            QString path = extDir.absoluteFilePath(fileName);
            auto findIt = std::find_if(extensions.begin(), extensions.end(), [path](ExtensionPtr& ext) {return ext->filePath == path;});
            if (findIt != extensions.end())
                continue; // already on the list

            ExtensionPtr ext = ExtensionPtr::create();
            ext->filePath = path;
            ext->initFunc = QString();
            ext->databases = QStringList();
            ext->allDatabases = false;
            extensions << ext;
            qDebug() << "SQLite extension:" << path;
        }
    }
}

void SqliteExtensionManagerImpl::storeInConfig()
{
    QVariantList list;
    QHash<QString,QVariant> extHash;
    for (ExtensionPtr& ext : extensions)
    {
        extHash["filePath"] = ext->filePath;
        extHash["initFunc"] = ext->initFunc;
        extHash["allDatabases"] = ext->allDatabases;
        extHash["databases"] = common(DBLIST->getDbNames(),  ext->databases);
        list << extHash;
    }
    CFG_CORE.Internal.Extensions.set(list);
}

void SqliteExtensionManagerImpl::loadFromConfig()
{
    extensions.clear();

    QVariantList list = CFG_CORE.Internal.Extensions.get();
    QHash<QString,QVariant> extHash;
    ExtensionPtr ext;
    for (const QVariant& var : list)
    {
        extHash = var.toHash();
        ext = ExtensionPtr::create();
        ext->filePath = extHash["filePath"].toString();
        ext->initFunc = extHash["initFunc"].toString();
        ext->databases = extHash["databases"].toStringList();
        ext->allDatabases = extHash["allDatabases"].toBool();
        extensions << ext;
        qDebug() << "SQLite extension from config:" << ext->filePath;
    }
}
