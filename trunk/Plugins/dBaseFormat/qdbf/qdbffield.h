#ifndef QDBFFIELD_H
#define QDBFFIELD_H

#include "qdbf_global.h"
#include <QtCore/QVariant>

namespace QDbf {

namespace Internal {
    class QDbfFieldPrivate;
    class QDbfTablePrivate;
}

class QDBF_EXPORT QDbfField
{
public:
    QDbfField(const QString &fieldName = QString::null, QVariant::Type type = QVariant::Invalid);
    QDbfField(const QDbfField &other);
    bool operator==(const QDbfField &other) const;
    inline bool operator!=(const QDbfField &other) const { return !operator==(other); }
    QDbfField &operator=(const QDbfField &other);
    ~QDbfField();

    enum QDbfType
    {
        UnknownDataType = -1,
        Character,
        Date,
        FloatingPoint,
        Logical,
        //Memo,
        Number
    };

    void setValue(const QVariant &value);
    inline QVariant value() const { return val; }

    void setName(const QString &name);
    QString name() const;

    bool isNull() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    void clear();

    void setType(QVariant::Type type);
    QVariant::Type type() const;

    void setQDbfType(QDbfType type);
    QDbfType dbfType() const;

    void setLength(int fieldLength);
    int length() const;

    void setPrecision(int precision);
    int precision() const;

    void setOffset(int offset);
    int offset() const;

    void setDefaultValue(const QVariant &value);
    QVariant defaultValue() const;

private:
    Internal::QDbfFieldPrivate *d;
    QVariant val;
    void detach();

    friend class Internal::QDbfTablePrivate;
};

} // namespace QDbf

QDebug operator<<(QDebug, const QDbf::QDbfField&);

#endif // QDBFFIELD_H
