#include "iconmanager.h"
#include "sqlitestudio.h"
#include "services/pluginmanager.h"
#include "common/unused.h"
#include "common/global.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QIcon>
#include <QMovie>
#include <QDebug>
#include <QPainter>

IconManager* IconManager::instance = nullptr;

IconManager* IconManager::getInstance()
{
    if (instance == nullptr)
        instance = new IconManager();

    return instance;
}

QString IconManager::getFilePathForName(const QString& name)
{
    return paths[name];
}

IconManager::IconManager()
{
}

void IconManager::init()
{
    Icon::init();

    iconDirs += qApp->applicationDirPath() + "/img";
    iconDirs += ":/icons";

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_ICONS");
    if (!envDirs.isNull())
        iconDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef ICONS_DIR
    iconDirs += STRINGIFY(ICONS_DIR);
#endif

    iconFileExtensions << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.svg" << "*.SVG";
    movieFileExtensions << "*.gif" << "*.GIF" << "*.mng" << "*.MNG";

    for (QString& dirPath : iconDirs)
    {
        loadRecurently(dirPath, "", false);
        loadRecurently(dirPath, "", true);
    }

    Icon::loadAll();

    if (PLUGINS->arePluginsInitiallyLoaded())
        enableRescanning();
    else
        connect(PLUGINS, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(pluginsInitiallyLoaded()));
}

QStringList IconManager::getIconDirs() const
{
    return iconDirs;
}

void IconManager::rescanResources(const QString& pluginName)
{
    if (!pluginName.isNull() && PLUGINS->isBuiltIn(pluginName))
        return;

    for (const QString& name : resourceMovies)
    {
        delete movies[name];
        movies.remove(name);
    }

    for (const QString& name : resourceIcons)
        icons.remove(name);

    resourceMovies.clear();
    resourceIcons.clear();
    loadRecurently(":/icons", "", true);
    loadRecurently(":/icons", "", false);

    Icon::reloadAll();
    emit rescannedFor(pluginName);
}

void IconManager::rescanResources(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(pluginType);
    rescanResources(plugin->getName());
}

void IconManager::pluginsAboutToMassUnload()
{
    disconnect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(rescanResources(Plugin*,PluginType*)));
    disconnect(PLUGINS, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(rescanResources(QString)));
}

void IconManager::pluginsInitiallyLoaded()
{
    Icon::reloadAll();
    enableRescanning();
    disconnect(PLUGINS, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(pluginsInitiallyLoaded()));
}

void IconManager::loadRecurently(QString dirPath, const QString& prefix, bool movie)
{
    QStringList extensions = movie ? movieFileExtensions : iconFileExtensions;
    QString path;
    QString name;
    QDir dir(dirPath);
    for (QFileInfo entry : dir.entryInfoList(extensions, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable))
    {
        if (entry.isDir())
        {
            loadRecurently(entry.absoluteFilePath(), prefix+entry.fileName()+"_", movie);
            continue;
        }

        path = entry.absoluteFilePath();
        name = entry.baseName();
        paths[name] = path;
        if (movie)
            movies[name] = new QMovie(path);
        else
            icons[name] = new QIcon(path);

        if (path.startsWith(":/"))
        {
            if (movie)
                resourceMovies << name;
            else
                resourceIcons << name;
        }
    }
}

void IconManager::enableRescanning()
{
    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(rescanResources(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(rescanResources(QString)));
    connect(PLUGINS, SIGNAL(aboutToQuit()), this, SLOT(pluginsAboutToMassUnload()));
}

QMovie* IconManager::getMovie(const QString& name)
{
    if (!movies.contains(name))
        qCritical() << "Movie missing:" << name;

    return movies[name];
}

QIcon* IconManager::getIcon(const QString& name)
{
    if (!icons.contains(name))
        qCritical() << "Icon missing:" << name;

    return icons[name];
}

bool IconManager::isMovie(const QString& name)
{
    return movies.contains(name);
}
