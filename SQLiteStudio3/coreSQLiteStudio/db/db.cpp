#include "db.h"
#include <QMetaEnum>
#include <QDebug>
#include <QDataStream>

Db::Db()
{
//    qDebug() << "Db::Db()" << this;
}

Db::~Db()
{
//    qDebug() << "Db::~Db()" << this;
}

void Db::metaInit()
{
    qRegisterMetaType<Db*>("Db*");
    qRegisterMetaTypeStreamOperators<Db*>("Db*");
}

QString Db::flagsToString(Db::Flags flags)
{
    int idx = staticMetaObject.indexOfEnumerator("Flag");
    if (idx == -1)
        return QString();

    QMetaEnum en = staticMetaObject.enumerator(idx);
    return en.valueToKeys(static_cast<int>(flags));
}

QDataStream &operator <<(QDataStream &out, const Db* myObj)
{
    out << reinterpret_cast<quint64>(myObj);
    return out;
}


QDataStream &operator >>(QDataStream &in, Db*& myObj)
{
    quint64 ptr;
    in >> ptr;
    myObj = reinterpret_cast<Db*>(ptr);
    return in;
}

QDebug operator<<(QDebug dbg, const Db* db)
{
    dbg.nospace() << "<DB:" << (db ? db->getName() : 0x0) << ">";
    return dbg.space();
}
