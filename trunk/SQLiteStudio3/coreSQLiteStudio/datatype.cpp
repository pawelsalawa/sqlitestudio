#include "datatype.h"
#include <QMetaEnum>

QList<DataType::Enum> DataType::values = [=]() -> QList<DataType::Enum>
{
    QList<DataType::Enum> list;
    QMetaEnum metaEnum = DataType::staticMetaObject.enumerator(0);
    DataType::Enum value;
    for (int i = 0; i < metaEnum.keyCount(); i++)
    {
        value = static_cast<DataType::Enum>(metaEnum.value(i));
        if (value == DataType::unknown)
            continue;

        list << value;
    }

    return list;
}();

const QStringList DataType::names = [=]() -> QStringList
{
    QStringList list;
    QMetaEnum metaEnum = DataType::staticMetaObject.enumerator(0);
    DataType::Enum value;
    for (int i = 0; i < metaEnum.keyCount(); i++)
    {
        value = static_cast<DataType::Enum>(metaEnum.value(i));
        if (value == DataType::unknown)
            continue;

        list << DataType::toString(value);
    }

    return list;
}();

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
        return unknown;

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
        case unknown:
            break;
    }
    return false;
}

bool DataType::isBinary(const QString& type)
{
    static const QStringList binaryTypes = {"BLOB", "CLOB", "LOB"};
    return binaryTypes.contains(type.toUpper());
}
