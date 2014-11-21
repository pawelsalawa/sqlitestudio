#include <QString>
#include <QList>
#include <QStringList>
#include <QtTest>
#include "tsvserializer.h"
#include "csvserializer.h"

// TODO Add tests for CsvSerializer

class DsvFormatsTestTest : public QObject
{
        Q_OBJECT

    public:
        DsvFormatsTestTest();

    private:
        QList<QStringList> sampleData;
        QString sampleTsv;

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testTsv1();
};

DsvFormatsTestTest::DsvFormatsTestTest()
{
}

void DsvFormatsTestTest::initTestCase()
{
    sampleData << QStringList{"a", "b c", "\"d\""};
    sampleData << QStringList{"a\"a\"", "\"b\"c\"", "d\"\"e"};
    sampleData << QStringList{"a\na", "b\tc", "d\t\"e"};

#ifdef Q_OS_MACX
    QString lineSep = "\r";
#else
    QString lineSep = "\n";
#endif

    sampleTsv = "";
    sampleTsv += "a\tb c\t\"d\"";
    sampleTsv += lineSep;
    sampleTsv += "a\"a\"\t\"b\"c\"\td\"\"e";
    sampleTsv += lineSep;
    sampleTsv += "\"a\na\"\t\"b\tc\"\t\"d\t\"\"e\"";
}

void DsvFormatsTestTest::cleanupTestCase()
{
}

void DsvFormatsTestTest::testTsv1()
{
    QString result = TsvSerializer::serialize(sampleData);

    QString common = "";
    int i;
    if (result != sampleTsv)
    {
        int lgt = qMax(result.length(), sampleTsv.length());
        for (i = 0; i < lgt && result[i] == sampleTsv[i]; i++)
            common.append(result[i]);
    }

    QVERIFY2(result == sampleTsv, QString("Mismatch after %1: %2\nSample: %3\nGot   : %4").arg(i).arg(common, sampleTsv, result).toLocal8Bit().data());
}

QTEST_APPLESS_MAIN(DsvFormatsTestTest)

#include "tst_dsvformatstesttest.moc"
