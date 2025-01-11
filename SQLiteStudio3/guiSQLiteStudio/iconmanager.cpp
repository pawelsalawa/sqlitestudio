#include "iconmanager.h"
#include "sqlitestudio.h"
#include "services/pluginmanager.h"
#include "common/unused.h"
#include "common/global.h"
#include "common/utils.h"
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
        loadRecurently(dirPath, "", false);

    Icon::loadAll();
    enableRescanning();
}

QStringList IconManager::getIconDirs() const
{
    return iconDirs;
}

void IconManager::rescanResources(const QString& pluginName)
{
    if (!pluginName.isNull() && PLUGINS->isBuiltIn(pluginName))
        return;

    QStringList pluginMovies = pluginResourceMovies[pluginName];
    pluginResourceMovies.remove(pluginName);
    for (const QString& name : pluginMovies)
    {
        movies[name]->deleteLater();
        movies.remove(name);
    }

    QStringList pluginIcons = pluginResourceIcons[pluginName];
    pluginResourceIcons.remove(pluginName);
    for (const QString& name : pluginIcons)
    {
        delete icons[name];
        icons.remove(name);
    }

    loadRecurently(":/icons", "", true);

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

void IconManager::loadRecurently(QString dirPath, const QString& prefix, bool onlyNew)
{
    loadRecurently(dirPath, prefix, true, onlyNew);
    loadRecurently(dirPath, prefix, false, onlyNew);
}

void IconManager::loadRecurently(QString dirPath, const QString& prefix, bool movie, bool onlyNew)
{
    QStringList extensions = movie ? movieFileExtensions : iconFileExtensions;
    QDir dir(dirPath);
    QFileInfoList entryList = dir.entryInfoList(extensions, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable);
    std::ranges::sort(entryList, [](const QFileInfo &e1, const QFileInfo &e2)
    {
        return e1.baseName() < e2.baseName();
    });

    for (QFileInfo entry : entryList)
    {
        if (entry.isDir())
        {
            loadRecurently(entry.absoluteFilePath(), prefix+entry.fileName()+"_", movie, onlyNew);
            continue;
        }

        QString path = entry.absoluteFilePath();
        QString name = entry.baseName();
        QString realName = name.contains("@") ? name.left(name.indexOf("@")) : name;
        if (icons.contains(realName))
        {
            if (!onlyNew)
                qWarning() << "Skipping icon" << name << "because it's already loaded, even though app is now loading all icons.";

            continue;
        }

        if (movie)
        {
            paths[name] = path;
            movies[name] = new QMovie(path);
        }
        else if (realName != name)
        {
            if (!icons.contains(realName))
            {
                qWarning() << "Failed to load additional icon size" << name << "because base size icon" << realName << "was not loaded.";
                continue;
            }
            QIcon* icon = icons[realName];
            int dim = name.mid(name.indexOf("@") + 1).toInt();
            QSize size = QSize(dim, dim);
            icon->addFile(path, size);
        }
        else
        {
            paths[name] = path;
            icons[name] = new QIcon(path);
            svgs[name] = entry.fileName().toLower().endsWith(".svg");
        }

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

bool IconManager::isSvg(const QString& name) const
{
    return svgs[name];
}

bool IconManager::isMovie(const QString& name)
{
    return movies.contains(name);
}
