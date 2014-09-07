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
#define DEF_ICO2(E,Src,Attr) Icon E = Icon::createFrom(#E, Src, Icon::Attr);
#define DEF_ICO3(E,Src) Icon E = Icon::aliasOf(#E, &Src);

class GUI_API_EXPORT Icon
{
    public:
        enum Attributes
        {
            NONE,
            PLUS,
            MINUS,
            EDIT,
            DELETE,
            DENIED,
            INFO,
            WARNING,
            QUESTION,
            ERROR,
            SORT_ASC,
            SORT_DESC
        };

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
        QMovie* toQMoviePtr() const;
        QVariant toQVariant() const;
        QIcon* with(Attributes attr);

        operator Icon*();
        operator QIcon() const;
        operator QIcon*() const;
        operator QPixmap() const;
        operator QMovie*() const;
        operator QVariant() const;

        static void init();
        static void loadAll();
        static void reloadAll();
        static Icon& createFrom(const QString& name, Icon* copy, Attributes attr);
        static Icon& aliasOf(const QString& name, Icon* other);
        static QIcon merge(const QIcon& icon, Attributes attr);

    private:
        explicit Icon(const QString& name);

        static QString getIconNameForAttribute(Attributes attr);
        static QIcon mergeAttribute(const QIcon* icon, Attributes attr);

        bool loaded = false;
        bool movie = false;
        QString name;
        Attributes attr = NONE;
        QString fileName;
        QString filePath;
        Icon* copyFrom = nullptr;
        Icon* aliased = nullptr;
        QMovie* movieHandle = nullptr;
        QIcon* iconHandle = nullptr;
        QHash<int,QIcon*> dynamicallyAttributed;

        static QHash<QString,Icon*> instances;
};

GUI_API_EXPORT QDataStream &operator<<(QDataStream &out, const Icon* icon);
GUI_API_EXPORT QDataStream &operator>>(QDataStream &in, const Icon*& icon);

Q_DECLARE_METATYPE(const Icon*)

#endif // ICONS_H
