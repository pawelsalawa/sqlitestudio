#include "icon.h"
#include "iconmanager.h"
#include "common/global.h"
#include <QPainter>
#include <QMovie>
#include <QDebug>
#include <QBuffer>
#include <QGuiApplication>
#include <QScreen>

QHash<QString,Icon*> Icon::instances;


Icon::Icon(const QString& name, const QString& fileName) :
    name(name)
{
    this->fileName = fileName;
    instances[name] = this;
}

Icon::Icon(const Icon& other) :
    loaded(other.loaded), movie(other.movie), name(other.name), filePath(other.filePath), copyFrom(other.copyFrom),
    aliased(other.aliased), movieHandle(other.movieHandle), iconHandle(other.iconHandle)
{
    instances[name] = this;
}

Icon::Icon(const QString& name) :
    name(name)
{
    instances[name] = this;
}

Icon::~Icon()
{
    safe_delete(iconHandle);
    safe_delete(movieHandle);
}

void Icon::load()
{
    if (aliased)
    {
        aliased->load();
        return;
    }

    if (loaded)
        return;

    if (copyFrom) // currently copyFrom works only on icons, not movies
    {
        if (!copyFrom->loaded)
            copyFrom->load();

        // Get base icon
        QIcon* icon = copyFrom->toQIconPtr();
        if (!icon)
        {
            qWarning() << "No QIcon in icon to copy from, while copying icon named" << copyFrom->name;
            return;
        }

        iconHandle = new QIcon(*icon);
    }
    else
    {
        filePath = IconManager::getInstance()->getFilePathForName(fileName);
        if (!filePath.isNull())
        {
            if (IconManager::getInstance()->isMovie(fileName))
                movieHandle = IconManager::getInstance()->getMovie(fileName);
            else
                iconHandle = IconManager::getInstance()->getIcon(fileName);
        }
        else
            qWarning() << "No file path for icon" << name;
    }

    loaded = true;
}

QString Icon::toImgSrc() const
{
    if (aliased)
        return aliased->toImgSrc();

    if (!filePath.isNull())
        return getPath();
    else
        return toBase64Url();
}

QString Icon::toBase64Url() const
{
    static const QString urlTempl = QStringLiteral("data:image/png;base64,%1");
    return urlTempl.arg(QString(toBase64()));
}

QByteArray Icon::toBase64() const
{
    return toPixmapBytes().toBase64();
}

QByteArray Icon::toPixmapBytes() const
{
    if (aliased)
        return aliased->toPixmapBytes();

    QByteArray byteArray;
    if (!loaded)
    {
        qCritical() << "Referring to a movie that was not yet loaded:" << name;
        return byteArray;
    }

    QBuffer buffer(&byteArray);
    iconHandle->pixmap(16, 16).save(&buffer, "PNG");
    return byteArray;
}

QString Icon::toUrl() const
{
    if (aliased)
        return aliased->toUrl();

    if (filePath.isNull())
        return toBase64Url();

    return filePath;
}

QIcon* Icon::toQIconPtr() const
{
    if (aliased)
        return aliased->toQIconPtr();

    if (!loaded)
    {
        qCritical() << "Referring to an icon that was not yet loaded:" << name;
        return nullptr;
    }

    return iconHandle;
}

QIcon Icon::toQIcon() const
{
    return *toQIconPtr();
}

Icon* Icon::toIconPtr()
{
    return this;
}

QPixmap Icon::toQPixmap() const
{
    qreal dpiRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
    return toQIconPtr()->pixmap(QSize(16, 16), dpiRatio);
}

QPixmap Icon::toQPixmap(int pixSize) const
{
    qreal dpiRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
    return toQIconPtr()->pixmap(QSize(pixSize, pixSize), dpiRatio);
}

QMovie* Icon::toQMoviePtr() const
{
    if (aliased)
        return aliased->toQMoviePtr();

    if (!loaded)
    {
        qCritical() << "Referring to a movie that was not yet loaded:" << name;
        return nullptr;
    }

    if (!movieHandle)
        return nullptr; // this is not a movie

    if (movieHandle->state() != QMovie::Running)
        movieHandle->start();

    return movieHandle;
}

QVariant Icon::toQVariant() const
{
    return QVariant::fromValue<QIcon>(operator QIcon());
}

Icon::operator Icon*()
{
    return this;
}

void Icon::init()
{
    qRegisterMetaType<const Icon*>();
}

QString Icon::getFileName() const
{
    return fileName;
}

QString Icon::getName() const
{
    return name;
}

QString Icon::getPath() const
{
    if (aliased)
        aliased->getPath();

    return filePath;
}

bool Icon::isNull() const
{
    if (aliased)
        return aliased->isNull();

    return (!iconHandle || iconHandle->isNull()) && !movieHandle;
}

bool Icon::isMovie() const
{
    if (aliased)
        return aliased->isMovie();

    return movieHandle != nullptr;
}

Icon& Icon::aliasOf(const QString& name, Icon* other)
{
    Icon* newIcon = new Icon(name);
    newIcon->aliased = other;
    newIcon->name = name;
    return *newIcon;
}

void Icon::loadAll()
{
    for (Icon* icon : instances.values())
        icon->load();
}

Icon::operator QVariant() const
{
    return toQVariant();
}

Icon::operator QMovie*() const
{
    return toQMoviePtr();
}

Icon::operator QIcon*() const
{
    return toQIconPtr();
}

Icon::operator QPixmap() const
{
    return toQPixmap();
}

Icon::operator QIcon() const
{
    return toQIcon();
}

QDataStream& operator<<(QDataStream& out, const Icon* icon)
{
    out << reinterpret_cast<qint64>(icon);
    return out;
}

QDataStream& operator>>(QDataStream& in, const Icon*& icon)
{
    qint64 ptr;
    in >> ptr;
    icon = reinterpret_cast<const Icon*>(ptr);
    return in;
}
