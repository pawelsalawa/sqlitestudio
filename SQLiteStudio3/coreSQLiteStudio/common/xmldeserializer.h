/*#ifndef XMLDESERIALIZER_H
#define XMLDESERIALIZER_H

#include "coreSQLiteStudio_global.h"
#include <QTextStream>
#include <QXmlStreamReader>
#include <QStack>
#include <QHash>

class API_EXPORT XmlDeserializer
{
    public:
        XmlDeserializer();

        QHash<QString,QVariant> deserialize(QIODevice* input);
        QHash<QString,QVariant> deserialize(const QString& input);

    private:
        QHash<QString,QVariant> deserialize(QXmlStreamReader& reader);
        void handleTokenType(QXmlStreamReader& reader, QXmlStreamReader::TokenType tokenType);
        void handleStartElement(QXmlStreamReader& reader);
        void handleText(QXmlStreamReader& reader);
        void handleEndElement(QXmlStreamReader& reader);

        QHash<QString, QVariant> output;
        QHash<QString, QVariant>* ctx;
        QStack<QHash<QString, QVariant>*> ctxStack;
};

#endif // XMLDESERIALIZER_H
*/
