#include "datatype.h"
#include <QMetaEnum>

QList<DataType::Enum> DataType::values()
{
    QList<Enum> list;
    QMetaEnum metaEnum = staticMetaObject.enumerator(0);
    Enum value;
    for (int i = 0; i < metaEnum.keyCount(); i++)
    {
        value = static_cast<Enum>(metaEnum.value(i));
        if (value == _NULL)
            continue;

        list << value;
    }

    return list;
}

QString DataType::toString(DataType::Enum e)
{
    QMetaEnum metaEnum = staticMetaObject.enumerator(0);
    const char* key = metaEnum.valueToKey(e);
    if (!key)
        return QString::null;

    return key;
}

DataType::Enum DataType::fromString(QString key, Qt::CaseSensitivity cs)
{
    QMetaEnum metaEnum = staticMetaObject.enumerator(0);

    if (cs == Qt::CaseInsensitive)
        key = key.toUpper();

    bool ok;
    Enum value = static_cast<Enum>(metaEnum.keyToValue(key.toLatin1().data(), &ok));
    if (!ok)
        return _NULL;

    return value;
}

bool DataType::isNumeric(DataType::Enum e)
{
    switch (e)
    {
        case BIGINT:
        case DECIMAL:
        case DOUBLE:
        case INTEGER:
        case INT:
        case NUMERIC:
        case REAL:
            return true;
        case BLOB:
        case BOOLEAN:
        case CHAR:
        case DATE:
        case DATETIME:
        case NONE:
        case STRING:
        case TEXT:
        case TIME:
        case VARCHAR:
        case _NULL:
            break;
    }
    return false;
}
