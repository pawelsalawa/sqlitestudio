#include "iconmanager.h"
#include "sqlitestudio.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QIcon>
#include <QMovie>

IconManager* IconManager::instance = nullptr;

IconManager* IconManager::getInstance()
{
    if (instance == nullptr)
        instance = new IconManager();

    return instance;
}

QIcon* IconManager::getIcon(const QString& name)
{
    if (icons.contains(name))
        return icons[name];

    Q_ASSERT_X(false, "IconManager", QString("Requested inexisting icon: %1").arg(name).toLatin1().data());
    return nullptr;
}

QMovie* IconManager::getMovie(const QString& name)
{
    if (movies.contains(name))
    {
        QMovie* movie = movies[name];
        if (movie->state() != QMovie::Running)
            movie->start();

        return movie;
    }

    Q_ASSERT_X(false, "IconManager", QString("Requested inexisting movie: %1").arg(name).toLatin1().data());
    return nullptr;
}

QString IconManager::getAbsoluteIconPath(const QString &name)
{
    if (iconPaths.contains(name))
        return iconPaths[name];

    Q_ASSERT_X(false, "IconManager", QString("Requested inexisting icon: %1").arg(name).toLatin1().data());
    return QString::null;
}

IconManager::IconManager()
{
    init();
}

void IconManager::init()
{
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
}

void IconManager::loadRecurently(QString dirPath, const QString& prefix, bool movie)
{
    QStringList extensions = movie ? movieFileExtensions : iconFileExtensions;
    QDir dir(dirPath);
    foreach (QFileInfo entry, dir.entryInfoList(extensions, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable))
    {
        if (entry.isDir())
        {
            loadRecurently(entry.absoluteFilePath(), prefix+entry.fileName()+"_", movie);
            continue;
        }
        if (movie)
            movies[entry.baseName()] = new QMovie(entry.absoluteFilePath());
        else
        {
            icons[entry.baseName()] = new QIcon(entry.absoluteFilePath());
            iconPaths[entry.baseName()] = entry.absoluteFilePath();
        }
    }
}
