/*#include "xmldeserializer.h"
#include <QDebug>

XmlDeserializer::XmlDeserializer()
{
}

QHash<QString, QVariant> XmlDeserializer::deserialize(QIODevice *input)
{
    QXmlStreamReader reader(input);
    return deserialize(reader);
}

QHash<QString, QVariant> XmlDeserializer::deserialize(const QString &input)
{
    QXmlStreamReader reader(input);
    return deserialize(reader);
}

QHash<QString, QVariant> XmlDeserializer::deserialize(QXmlStreamReader &reader)
{
    ctxStack.clear();
    output.clear();
    ctx = &output;

    QXmlStreamReader::TokenType tokenType;
    while ((tokenType = reader.readNext()) != QXmlStreamReader::EndDocument)
        handleTokenType(reader, tokenType);

    return output;
}

void XmlDeserializer::handleTokenType(QXmlStreamReader& reader, QXmlStreamReader::TokenType tokenType)
{
    switch (tokenType)
    {
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::ProcessingInstruction:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EntityReference:
            break;
        case QXmlStreamReader::Invalid:
            qDebug() << "Invalid token while parsing XML:" << reader.errorString();
            break;
        case QXmlStreamReader::StartElement:
            handleStartElement(reader);
            break;
        case QXmlStreamReader::Characters:
            handleText(reader);
            break;
        case QXmlStreamReader::EndElement:
            handleEndElement(reader);
            break;
    }
}

void XmlDeserializer::handleStartElement(QXmlStreamReader &reader)
{
    QString key = reader.name().toString();
    QHash<QString, QVariant> newCtx;
    ctx->insertMulti(key, newCtx);

    for (const QXmlStreamAttribute& attr : reader.attributes())
        ctx->insertMulti(attr.name().toString(), attr.value().toString());

    ctxStack.push(ctx);
    ctx = &((*ctx)[key]);
}

void XmlDeserializer::handleText(QXmlStreamReader &reader)
{
    ctx->insertMulti(QString(), reader.text().toString());
}

void XmlDeserializer::handleEndElement(QXmlStreamReader &reader)
{
    Q_UNUSED(reader);
    ctx = ctxStack.pop();
}
*/
