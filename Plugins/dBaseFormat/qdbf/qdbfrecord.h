#ifndef QDBFRECORD_H
#define QDBFRECORD_H

#include "qdbf_global.h"
class QString;

namespace QDbf {

class QDbfField;

namespace Internal {
    class QDbfRecordPrivate;
}

class QDBF_EXPORT QDbfRecord
{
public:
    QDbfRecord();
    QDbfRecord(const QDbfRecord &other);
    bool operator==(const QDbfRecord &other) const;
    inline bool operator!=(const QDbfRecord &other) const { return !operator==(other); }
    QDbfRecord &operator=(const QDbfRecord &other);
    ~QDbfRecord();

    void setRecordIndex(int index);
    int recordIndex() const;

    void setValue(int i, const QVariant &val);
    QVariant value(int i) const;

    void setValue(const QString &name, const QVariant &val);
    QVariant value(const QString &name) const;

    void setNull(int i);
    bool isNull(int i) const;

    void setNull(const QString &name);
    bool isNull(const QString &name) const;

    int indexOf(const QString &name) const;
    QString fieldName(int i) const;

    QDbfField field(int i) const;
    QDbfField field(const QString &name) const;

    void append(const QDbfField &field);
    void replace(int pos, const QDbfField &field);
    void insert(int pos, const QDbfField &field);
    void remove(int pos);

    bool isEmpty() const;

    void setDeleted(bool deleted);
    bool isDeleted() const;

    bool contains(const QString &name) const;
    void clear();
    void clearValues();
    int count() const;

private:
    Internal::QDbfRecordPrivate *d;
    void detach();
};

} // namespace QDbf

QDebug operator<<(QDebug, const QDbf::QDbfRecord&);

#endif // QDBFRECORD_H
