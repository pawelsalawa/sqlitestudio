#include "icon.h"
#include "iconmanager.h"
#include "common/global.h"
#include <QPainter>
#include <QMovie>
#include <QDebug>
#include <QBuffer>

QHash<QString,Icon*> Icon::instances;


Icon::Icon(const QString& name, const QString& fileName) :
    name(name)
{
    this->fileName = fileName;
    instances[name] = this;
}

Icon::Icon(const Icon& other) :
    loaded(other.loaded), movie(other.movie), name(other.name), attr(other.attr), filePath(other.filePath), copyFrom(other.copyFrom),
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
    for (QIcon* icon : dynamicallyAttributed.values())
        delete icon;

    dynamicallyAttributed.clear();

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

        iconHandle = new QIcon(mergeAttribute(icon, attr));
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
    return toQIconPtr()->pixmap(16, 16);
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

QIcon* Icon::with(Icon::Attributes attr)
{
    if (dynamicallyAttributed.contains(attr))
        return dynamicallyAttributed[attr];

    if (aliased)
        return aliased->with(attr);

    if (!loaded)
    {
        qCritical() << "Referring to a icon that was not yet loaded:" << name;
        return nullptr;
    }

    if (movieHandle)
        return nullptr; // this is a movie

    QIcon* merged = new QIcon(mergeAttribute(iconHandle, attr));
    dynamicallyAttributed[attr] = merged;
    return merged;
}

Icon::operator Icon*()
{
    return this;
}

void Icon::init()
{
    qRegisterMetaType<const Icon*>();
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<const Icon*>();
#else
    // Qt 6 does it automatically
#endif
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

Icon& Icon::createFrom(const QString& name, Icon* copy, Icon::Attributes attr)
{
    Icon* newIcon = new Icon(name);
    newIcon->copyFrom = copy;
    newIcon->attr = attr;
    newIcon->name = name;

    return *newIcon;
}

Icon& Icon::aliasOf(const QString& name, Icon* other)
{
    Icon* newIcon = new Icon(name);
    newIcon->aliased = other;
    newIcon->name = name;
    return *newIcon;
}

QIcon Icon::merge(const QIcon& icon, Icon::Attributes attr)
{
    return mergeAttribute(&icon, attr);
}

void Icon::loadAll()
{
    for (Icon* icon : instances.values())
        icon->load();
}

void Icon::reloadAll()
{
    for (Icon* icon : instances.values())
    {
        icon->loaded = false;
        icon->load();
    }
}

QString Icon::getIconNameForAttribute(Icon::Attributes attr)
{
    switch (attr)
    {
        case PLUS:
            return "plus_small";
        case MINUS:
            return "minus_small";
        case EDIT:
            return "edit_small";
        case DELETE:
            return "delete_small";
        case DENIED:
            return "denied_small";
        case INFO:
            return "info_small";
        case WARNING:
            return "warn_small";
        case QUESTION:
            return "question_small";
        case ERROR:
            return "error_small";
        case SORT_ASC:
            return "sort_ind_asc";
        case SORT_DESC:
            return "sort_ind_desc";
        case DISK:
            return "disk_small";
        case LIGHTENING:
            return "lightning_small";
        default:
            qWarning() << "Unhandled icon attribute:" << attr;
    }
    return QString();
}

QIcon Icon::mergeAttribute(const QIcon* icon, Icon::Attributes attr)
{
    QString attribName = getIconNameForAttribute(attr);
    QIcon* attrIcon = IconManager::getInstance()->getIcon(attribName);
    if (!attrIcon)
    {
        qWarning() << "No attribute icon for attribute:" << attribName;
        return *icon;
    }

    // Merge icons
    QPixmap attrPixmap = attrIcon->pixmap(16, 16);
    QPixmap newPixmap = icon->pixmap(16, 16);

    QPainter painter(&newPixmap);
    painter.drawPixmap(0, 0, attrPixmap);

    // Create new icon
    return QIcon(newPixmap);
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
