#ifndef CSVSERIALIZER_H
#define CSVSERIALIZER_H

#include "coreSQLiteStudio_global.h"
#include "csvformat.h"
#include <QTextStream>

class API_EXPORT CsvSerializer
{
    public:
        static QString serialize(const QList<QStringList>& data, const CsvFormat& format);
        static QString serialize(const QStringList& data, const CsvFormat& format);
        static QList<QStringList> deserialize(const QString& data, const CsvFormat& format);
        static QList<QList<QByteArray>> deserialize(const QByteArray& data, const CsvFormat& format);
        static QList<QStringList> deserialize(QTextStream& data, const CsvFormat& format);
        static QStringList deserializeOneEntry(QTextStream& data, const CsvFormat& format);
};

#endif // CSVSERIALIZER_H
