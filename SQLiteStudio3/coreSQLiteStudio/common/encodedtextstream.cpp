#include "encodedtextstream.h"
#include <QTextCodec>

#if QT_VERSION >= 0x060000

void EncodedTextStream::setCodec(QTextCodec *codec)
{
    this->codec = codec;
    hasCodec = true;
    setEncoding(QStringConverter::Latin1);
    setAutoDetectUnicode(false);
}

void EncodedTextStream::setCodec(const char *codecName)
{
    setCodec(QTextCodec::codecForName(codecName));
}

QString EncodedTextStream::readLine(qint64 maxlen)
{
    QString s;
    while ((maxlen ? s.length() < maxlen : true) && !atEnd()) {
        QChar c;
        *this >> c;
        if (c == '\n') {
            if (s.endsWith('\r'))
                s.chop(1);
            break;
        }
        s.append(c);
    }
    return s;
}

EncodedTextStream &EncodedTextStream::operator>>(char &c)
{
    this->QTextStream::operator>>(c);
    return *this;
}

EncodedTextStream &EncodedTextStream::operator>>(QChar &c)
{
    char ch;
    if (hasCodec) {
        QTextDecoder decoder = QTextDecoder(codec);
        QString s;
        do {
            QByteArray arr = QByteArray(" ");
            this->QTextStream::operator>>(arr[0]);
            s = decoder.toUnicode(arr);
            if (decoder.hasFailure()) {
                c = QChar::ReplacementCharacter;
                return *this;
            }
        } while (decoder.needsMoreData() && !atEnd());
        if (!s.isEmpty())
            c = s[0];
        else {
            if (decoder.needsMoreData())
                setStatus(QTextStream::ReadPastEnd);
            c = QChar::ReplacementCharacter;
        }
    } else
        this->QTextStream::operator>>(ch);
    return *this;
}

#endif
