#include "qdbffield.h"
#include "qdbfrecord.h"
#include <QDebug>
#include <QVariant>
#include <QVector>

namespace QDbf {
namespace Internal {

class QDbfRecordPrivate
{
public:
    QDbfRecordPrivate();
    QDbfRecordPrivate(const QDbfRecordPrivate &other);

    inline bool contains(int index) { return index >= 0 && index < fields.count(); }

    QAtomicInt ref;
    int index;
    bool isDeleted;
    QVector<QDbfField> fields;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfRecordPrivate::QDbfRecordPrivate() :
    ref(1), index(-1), isDeleted(false)
{
}

QDbfRecordPrivate::QDbfRecordPrivate(const QDbfRecordPrivate &other) :
    ref(1), index(other.index), isDeleted(other.isDeleted), fields(other.fields)
{
}

QDbfRecord::QDbfRecord() :
    d(new QDbfRecordPrivate())
{
}

QDbfRecord::QDbfRecord(const QDbfRecord &other) :
    d(other.d)
{
    d->ref.ref();
}

QDbfRecord &QDbfRecord::operator=(const QDbfRecord &other)
{
    if (this == &other) return *this;
    qAtomicAssign(d, other.d);
    return *this;
}

bool QDbfRecord::operator==(const QDbfRecord &other) const
{
    return (recordIndex() == other.recordIndex() &&
            isDeleted() == other.isDeleted() &&
            d->fields == other.d->fields);
}

QDbfRecord::~QDbfRecord()
{
    if (!d->ref.deref()) {
        delete d;
    }
}

void QDbfRecord::setRecordIndex(int index)
{
    d->index = index;
}

int QDbfRecord::recordIndex() const
{
    return d->index;
}

void QDbfRecord::setValue(int index, const QVariant &val)
{
    if (!d->contains(index)) {
        return;
    }

    detach();
    d->fields[index].setValue(val);
}

QVariant QDbfRecord::value(int index) const
{
    return d->fields.value(index).value();
}

void QDbfRecord::setValue(const QString &name, const QVariant &val)
{
    setValue(indexOf(name), val);
}

QVariant QDbfRecord::value(const QString &name) const
{
    return value(indexOf(name));
}

void QDbfRecord::setNull(int index)
{
    if (!d->contains(index)) {
        return;
    }

    detach();
    d->fields[index].clear();
}

bool QDbfRecord::isNull(int index) const
{
    return d->fields.value(index).isNull();
}

void QDbfRecord::setNull(const QString &name)
{
    setNull(indexOf(name));
}

bool QDbfRecord::isNull(const QString &name) const
{
    return isNull(indexOf(name));
}

int QDbfRecord::indexOf(const QString &name) const
{
    QString nm = name.toUpper();

    for (int i = 0; i < count(); ++i) {
        if (d->fields.at(i).name().toUpper() == nm) return i;
    }

    return -1;
}

QString QDbfRecord::fieldName(int index) const
{
    return d->fields.value(index).name();
}

QDbfField QDbfRecord::field(int index) const
{
    return d->fields.value(index);
}

QDbfField QDbfRecord::field(const QString &name) const
{
    return field(indexOf(name));
}

void QDbfRecord::append(const QDbfField &field)
{
    detach();
    d->fields.append(field);
}

void QDbfRecord::replace(int pos, const QDbfField &field)
{
    if (!d->contains(pos)) {
        return;
    }

    detach();
    d->fields[pos] = field;
}

void QDbfRecord::insert(int pos, const QDbfField &field)
{
    detach();
    d->fields.insert(pos, field);
}

void QDbfRecord::remove(int pos)
{
    if (!d->contains(pos)) {
        return;
    }

    detach();
    d->fields.remove(pos);
}

bool QDbfRecord::isEmpty() const
{
    return d->fields.isEmpty();
}

void QDbfRecord::setDeleted(bool deleted)
{
    detach();
    d->isDeleted = deleted;
}

bool QDbfRecord::isDeleted() const
{
    return d->isDeleted;
}

bool QDbfRecord::contains(const QString &name) const
{
    return indexOf(name) >= 0;
}

void QDbfRecord::clear()
{
    detach();
    d->fields.clear();
}

void QDbfRecord::clearValues()
{
    detach();
    int count = d->fields.count();
    for (int i = 0; i < count; ++i) {
        d->fields[i].clear();
    }
}

int QDbfRecord::count() const
{
    return d->fields.count();
}

void QDbfRecord::detach()
{
    qAtomicDetach(d);
}

QDebug operator<<(QDebug debug, const QDbfRecord &record)
{
    debug.nospace() << "QDbfRecord(" << record.count() << ')';

    for (int i = 0; i < record.count(); ++i) {
        debug.nospace() << '\n' << QString::fromLatin1("%1:").arg(i, 2)
                        << record.field(i) << record.value(i).toString();
    }

    return debug.space();
}
