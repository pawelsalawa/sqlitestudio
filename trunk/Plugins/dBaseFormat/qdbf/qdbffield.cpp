#include "qdbffield.h"

#include <QtCore/QDebug>

namespace QDbf {
namespace Internal {

class QDbfFieldPrivate
{
public:
    QDbfFieldPrivate(const QString &name, QVariant::Type type);
    QDbfFieldPrivate(const QDbfFieldPrivate &other);
    bool operator==(const QDbfFieldPrivate &other) const;

    QAtomicInt ref;
    QString name;
    QVariant::Type type;
    QDbfField::QDbfType dbfType;
    bool readOnly;
    int length;
    int precision;
    int offset;
    QVariant defaultValue;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfFieldPrivate::QDbfFieldPrivate(const QString &name, QVariant::Type type) :
    ref(1),
    name(name),
    type(type),
    dbfType(QDbfField::UnknownDataType),
    readOnly(false),
    length(-1),
    precision(-1),
    offset(0)
{
}

QDbfFieldPrivate::QDbfFieldPrivate(const QDbfFieldPrivate &other) :
    ref(1),
    name(other.name),
    type(other.type),
    dbfType(other.dbfType),
    readOnly(other.readOnly),
    length(other.length),
    precision(other.precision),
    offset(other.offset),
    defaultValue(other.defaultValue)
{
}

bool QDbfFieldPrivate::operator==(const QDbfFieldPrivate &other) const
{
    return (name == other.name &&
            type == other.type &&
            dbfType == other.dbfType &&
            readOnly == other.readOnly &&
            length == other.length &&
            precision == other.precision &&
            offset == other.offset &&
            defaultValue == other.defaultValue);
}

QDbfField::QDbfField(const QString &fieldName, QVariant::Type type) :
    d(new QDbfFieldPrivate(fieldName, type))
{
}

QDbfField::QDbfField(const QDbfField &other) :
    d(other.d)
{
    d->ref.ref();
    val = other.val;
}

bool QDbfField::operator==(const QDbfField &other) const
{
    return ((d == other.d || *d == *other.d) && val == other.val);
}

QDbfField &QDbfField::operator=(const QDbfField &other)
{
    if (this == &other) {
        return *this;
    }

    qAtomicAssign(d, other.d);
    val = other.val;

    return *this;
}

QDbfField::~QDbfField()
{
    if (!d->ref.deref()) {
        delete d;
        d = 0;
    }
}

void QDbfField::setValue(const QVariant &value)
{
    if (isReadOnly()) {
        return;
    }

    val = value;
}

void QDbfField::setName(const QString &name)
{
    detach();
    d->name = name;
}

QString QDbfField::name() const
{
    return d->name;
}

bool QDbfField::isNull() const
{
    return val.isNull();
}

void QDbfField::setReadOnly(bool readOnly)
{
    detach();
    d->readOnly = readOnly;
}

bool QDbfField::isReadOnly() const
{
    return d->readOnly;
}

void QDbfField::clear()
{
    if (isReadOnly()) {
        return;
    }

    val = QVariant(type());
}

void QDbfField::setType(QVariant::Type type)
{
    detach();
    d->type = type;
}

QVariant::Type QDbfField::type() const
{
    return d->type;
}

void QDbfField::setQDbfType(QDbfType type)
{
    detach();
    d->dbfType = type;
}

QDbfField::QDbfType QDbfField::dbfType() const
{
    return d->dbfType;
}

void QDbfField::setLength(int fieldLength)
{
    detach();
    d->length = fieldLength;
}

int QDbfField::length() const
{
    return d->length;
}

void QDbfField::setPrecision(int precision)
{
    detach();
    d->precision = precision;
}

int QDbfField::precision() const
{
    return d->precision;
}

void QDbfField::setOffset(int offset)
{
    detach();
    d->offset = offset;
}

int QDbfField::offset() const
{
    return d->offset;
}

void QDbfField::setDefaultValue(const QVariant &value)
{
    detach();
    d->defaultValue = value;
}

QVariant QDbfField::defaultValue() const
{
    return d->defaultValue;
}

void QDbfField::detach()
{
    qAtomicDetach(d);
}

QDebug operator<<(QDebug debug, const QDbfField &field)
{
    debug.nospace() << "QDbfField("
                    << field.name() << ", "
                    << QVariant::typeToName(field.type());

    if (field.length() >= 0) {
        debug.nospace() << ", length: " << field.length();
    }

    if (field.precision() >= 0) {
        debug.nospace() << ", precision: " << field.precision();
    }

    if (!field.defaultValue().isNull()) {
        debug.nospace() << ", auto-value: \"" << field.defaultValue() << '\"';
    }

    debug.nospace() << ')';

    return debug.space();
}
