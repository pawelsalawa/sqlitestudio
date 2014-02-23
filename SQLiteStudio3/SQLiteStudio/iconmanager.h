#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QStringList>
#include <QHash>
#include <QIcon>

class QMovie;

class IconManager
{
    public:
        static IconManager* getInstance();

        QIcon* getIcon(const QString& name);
        QMovie* getMovie(const QString& name);
        QString getAbsoluteIconPath(const QString& name);

    private:
        IconManager();
        void init();
        void loadRecurently(QString dirPath, const QString& prefix, bool movie);

        static IconManager* instance;
        QHash<QString,QIcon*> icons;
        QHash<QString,QMovie*> movies;
        QHash<QString,QString> iconPaths;
        QStringList iconDirs;
        QStringList iconFileExtensions;
        QStringList movieFileExtensions;
};

#define ICON(x) *(IconManager::getInstance()->getIcon(x))
#define ICON_PIX(x) IconManager::getInstance()->getIcon(x)->pixmap(16, 16)
#define ICON_VARIANT(x) QVariant::fromValue<QIcon>(*(IconManager::getInstance()->getIcon(x)))
#define ICON_PTR(x) IconManager::getInstance()->getIcon(x)
#define MOVIE(x) IconManager::getInstance()->getMovie(x)
#define ICON_PATH(x) IconManager::getInstance()->getAbsoluteIconPath(x)

#endif // ICONMANAGER_H
