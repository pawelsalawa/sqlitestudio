#ifndef BIGDEC_H
#define BIGDEC_H

#include <QString>
#include "coreSQLiteStudio_global.h"

class API_EXPORT BigDec
{
    public:
        BigDec();
        BigDec(const QString& s);

        void fromString(const QString& s);
        BigDec& operator+=(int value);
        BigDec& operator+=(uint value);
        BigDec& operator+=(qlonglong value);
        BigDec& operator+=(qulonglong value);
        BigDec& operator+=(double value);
        BigDec& operator+=(const QString& otherStr);
        QString toString() const;

    private:
        static QString stripLeadingZeros(const QString& s);
        static QString stripTrailingZeros(QString s, int& scale);
        static void alignScales(QString& a, int& scaleA, QString& b, int& scaleB);
        static int compareAbs(const QString& a, const QString& b);
        static QString addStrings(const QString& a, const QString& b);
        static QString subStrings(const QString& a, const QString& b);
        void normalize();

        QString digits;
        int scale;
        bool negative;
};

#endif // BIGDEC_H
