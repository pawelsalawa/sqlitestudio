#ifndef DATATYPE_H
#define DATATYPE_H

#include "coreSQLiteStudio_global.h"
#include <QObject>
#include <QVariant>

class API_EXPORT DataType : public QObject
{
    Q_OBJECT

    public:
        enum Enum
        {
            ANY,
            BIGINT,
            BLOB,
            BOOLEAN,
            CHAR,
            DATE,
            DATETIME,
            DECIMAL,
            DOUBLE,
            INTEGER,
            INT,
            NONE,
            NUMERIC,
            REAL,
            STRING,
            TEXT,
            TIME,
            VARCHAR,
            unknown
        };
        Q_ENUM(Enum)

        DataType();
        DataType(const QString& fullTypeString);
        DataType(const QString& type, const QVariant& scale, const QVariant& precision);
        DataType(const DataType& other);
        Enum getType() const;
        void setType(Enum value);
        QVariant getPrecision() const;
        void setPrecision(const QVariant& value);
        QVariant getScale() const;
        void setScale(const QVariant& value);
        QString toString() const;
        QString toFullTypeString() const;
        void setEmpty();
        bool isNumeric() const;
        bool isBinary() const;
        bool isStrict() const;
        bool isNull() const;
        bool isEmpty() const;
        DataType& operator=(const DataType& other);

        static QString toString(Enum e);
        static Enum fromString(QString key, Qt::CaseSensitivity cs = Qt::CaseSensitive);
        static bool isNumeric(Enum e);
        static bool isStrict(Enum e);
        static bool isStrict(const QString& type);
        static bool isBinary(const QString& type);
        static QList<Enum> getAllTypes();
        static QStringList getAllNames();
        static QList<Enum> getAllTypesForUiDropdown();
        static QList<Enum> getStrictValues();
        static QStringList getStrictValueNames();

    private:
        Enum type = unknown;
        QVariant precision;
        QVariant scale;
        QString typeStr;

        static QList<Enum> values;
        static const QStringList names;
        static QList<Enum> valuesForUiDropdown;
        static QList<Enum> strictValues;
        static const QStringList strictNames;
};

#endif // DATATYPE_H
