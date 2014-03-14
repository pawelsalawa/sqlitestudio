#include "db.h"
#include <QMetaEnum>

Db::Db()
{
}

Db::~Db()
{
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
        return QString::null;

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
