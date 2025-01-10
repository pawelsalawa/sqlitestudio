#ifndef ICONS_H
#define ICONS_H

#include "guiSQLiteStudio_global.h"
#include <QString>
#include <QIcon>
#include <QVariant>

class QMovie;

#define DEF_ICONS(TypeName, ObjName, Defs) \
    struct GUI_API_EXPORT TypeName \
    { \
        Defs \
    }; \
    TypeName ObjName;

#define DEF_ICON(E,N) Icon E = Icon(#E, N);
#define DEF_ICO3(E,Src) Icon E = Icon::aliasOf(#E, &Src);

class GUI_API_EXPORT Icon
{
    public:
        Icon(const QString& name, const QString& fileName);
        Icon(const Icon& other);
        ~Icon();

        QString getFileName() const;
        QString getName() const;
        QString getPath() const;
        bool isNull() const;
        bool isMovie() const;
        void load();
        QString toImgSrc() const;
        QString toBase64Url() const;
        QByteArray toBase64() const;
        QByteArray toPixmapBytes() const;
        QString toUrl() const;
        QIcon* toQIconPtr() const;
        QIcon toQIcon() const;
        Icon* toIconPtr();
        QPixmap toQPixmap() const;
        QPixmap toQPixmap(int pixSize) const;
        QMovie* toQMoviePtr() const;
        QVariant toQVariant() const;

        operator Icon*();
        operator QIcon() const;
        operator QIcon*() const;
        operator QPixmap() const;
        operator QMovie*() const;
        operator QVariant() const;

        static void init();
        static void loadAll();
        static Icon& aliasOf(const QString& name, Icon* other);

    private:
        explicit Icon(const QString& name);

        bool loaded = false;
        bool movie = false;
        QString name;
        QString fileName;
        QString filePath;
        Icon* copyFrom = nullptr;
        Icon* aliased = nullptr;
        QMovie* movieHandle = nullptr;
        QIcon* iconHandle = nullptr;

        static QHash<QString,Icon*> instances;
};

GUI_API_EXPORT QDataStream &operator<<(QDataStream &out, const Icon* icon);
GUI_API_EXPORT QDataStream &operator>>(QDataStream &in, const Icon*& icon);

Q_DECLARE_METATYPE(const Icon*)

#endif // ICONS_H
