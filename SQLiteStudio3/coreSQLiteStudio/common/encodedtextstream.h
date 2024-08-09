#ifndef ENCODEDTEXTSTREAM_H
#define ENCODEDTEXTSTREAM_H

#include "coreSQLiteStudio_global.h"

#include <QTextStream>

class QTextCodec;

class API_EXPORT EncodedTextStream : public QTextStream
{
    using QTextStream::QTextStream;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    public:
        /* This class re-implements in Qt 6 a limited subset of
         * QTextCodec-related Qt 5 QTextStream functionality. Only these
         * methods are supported: */
        void setCodec(const char *codecName);
        QString readLine(qint64 maxlen = 0);
        EncodedTextStream &operator>>(char &c);
        EncodedTextStream &operator>>(QChar &c);
    private:
        QTextCodec *codec;
        bool hasCodec = false;

        void setCodec(QTextCodec *codec);
#endif
};

#endif // ENCODEDTEXTSTREAM_H
