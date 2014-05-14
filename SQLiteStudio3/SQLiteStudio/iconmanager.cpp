#include "iconmanager.h"
#include "sqlitestudio.h"
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

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_ICONS");
    if (!envDirs.isNull())
        iconDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef ICONS_DIR
    iconDirs += ICONS_DIR;
#endif

    iconFileExtensions << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.svg" << "*.SVG";
    movieFileExtensions << "*.gif" << "*.GIF" << "*.mng" << "*.MNG";

    foreach (QString dirPath, iconDirs)
    {
        loadRecurently(dirPath, "", false);
        loadRecurently(dirPath, "", true);
    }

    Icon::loadAll();
}

void IconManager::loadRecurently(QString dirPath, const QString& prefix, bool movie)
{
    QStringList extensions = movie ? movieFileExtensions : iconFileExtensions;
    QString path;
    QString name;
    QDir dir(dirPath);
    foreach (QFileInfo entry, dir.entryInfoList(extensions, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable))
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
    }
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
