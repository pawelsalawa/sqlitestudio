#ifndef CSVSERIALIZER_H
#define CSVSERIALIZER_H

#include "coreSQLiteStudio_global.h"
#include "csvformat.h"

class API_EXPORT CsvSerializer
{
    public:
        static QString serialize(const QList<QStringList>& data, const CsvFormat& format);
        static QList<QStringList> deserialize(const QString& data, const CsvFormat& format);
};

#endif // CSVSERIALIZER_H
