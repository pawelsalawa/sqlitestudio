#ifndef DATATYPE_H
#define DATATYPE_H

#include "coreSQLiteStudio_global.h"
#include <QObject>

class API_EXPORT DataType : public QObject
{
        Q_OBJECT
        Q_ENUMS(Enum)

    public:
        enum Enum
        {
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

        static QString toString(Enum e);
        static Enum fromString(QString key, Qt::CaseSensitivity cs = Qt::CaseSensitive);
        static bool isNumeric(Enum e);

        static QList<Enum> values;
        static const QStringList names;
};

#endif // DATATYPE_H
