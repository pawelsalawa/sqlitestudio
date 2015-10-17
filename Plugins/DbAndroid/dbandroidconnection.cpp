#include "dbandroidconnection.h"
#include <QDebug>

QByteArray DbAndroidConnection::convertBlob(const QString& value)
{
    if (!value.startsWith("X'", Qt::CaseInsensitive) || !value.endsWith("'"))
    {
        qCritical() << "Invalid BLOB value from Android. Doesn't match BLOB pattern:" << value;
        return QByteArray();
    }

    return QByteArray::fromHex(value.mid(2, value.length() - 3).toLatin1());
}

