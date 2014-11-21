#ifndef TSVSERIALIZER_H
#define TSVSERIALIZER_H

#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include <QStringList>

class API_EXPORT TsvSerializer
{
    public:
        static QString serialize(const QList<QStringList>& data);
        static QString serialize(const QStringList& data);
        static QList<QStringList> deserialize(const QString& data);

    private:
        static QString rowSeparator;
        static QString columnSeparator;
};

#endif // TSVSERIALIZER_H
