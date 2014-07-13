#include "dbversionconverter.h"
#include "parser/lexer.h"
#include "parser/keywords.h"
#include <QString>
#include <QtTest>

class DbVersionConverterTestTest : public QObject
{
        Q_OBJECT

    public:
        DbVersionConverterTestTest();

    private Q_SLOTS:
        void initTestCase();
        void init();
        void cleanupTestCase();
        void testColumnAutoIncr();
        void testIndexedColumn();
        void testInsertMultiValues();
        void testSelectWith();
        void testTableWithoutRowId();
        void testTableWithDefaultCtime();

    private:
        void printErrors();

        DbVersionConverter* converter = nullptr;
};

DbVersionConverterTestTest::DbVersionConverterTestTest()
{
}

void DbVersionConverterTestTest::testColumnAutoIncr()
{
    QString query = "CREATE TABLE test (col INTEGER PRIMARY KEY AUTOINCREMENT);";
    QString result = converter->convert3To2(query);

    printErrors();
    QVERIFY(converter->getErrors().size() == 0);
    QVERIFY(result == "CREATE TABLE test (col INTEGER PRIMARY KEY);");
}

void DbVersionConverterTestTest::testIndexedColumn()
{
    QString query = "CREATE INDEX idx ON test (col COLLATE NOCASE ASC);";
    QString result = converter->convert3To2(query);

    printErrors();
    QVERIFY(converter->getErrors().size() == 0);
    QVERIFY(result == "CREATE INDEX idx ON test (col ASC);");
}

void DbVersionConverterTestTest::testInsertMultiValues()
{
    QString query = "INSERT INTO test (col1, col2) VALUES (1, 'a'), (2, 'b');";
    QString result = converter->convert3To2(query);

    printErrors();
    QVERIFY(converter->getErrors().size() == 0);
    QVERIFY(result == "INSERT INTO test (col1, col2) SELECT 1, 'a' UNION ALL SELECT 2, 'b';");
}

void DbVersionConverterTestTest::testSelectWith()
{
    QString query = "WITH RECURSIVE cnt (x) AS (VALUES(1) UNION ALL SELECT x + 1 FROM cnt WHERE x < 1000000) SELECT x FROM cnt;";
    QString result = converter->convert3To2(query);

    QVERIFY(converter->getErrors().size() == 1);
    QVERIFY(result == ";");
}

void DbVersionConverterTestTest::testTableWithoutRowId()
{
    QString query = "CREATE TABLE test (col PRIMARY KEY) WITHOUT ROWID;";
    QString result = converter->convert3To2(query);

    printErrors();
    QVERIFY(converter->getErrors().size() == 0);
    QVERIFY(result == "CREATE TABLE test (col PRIMARY KEY);");
}

void DbVersionConverterTestTest::testTableWithDefaultCtime()
{
    QString query = "CREATE TABLE test (col INT DEFAULT current_date NOT NULL);";
    QString result = converter->convert3To2(query);

    printErrors();
    QVERIFY(converter->getErrors().size() == 0);
    QVERIFY(result == "CREATE TABLE test (col INT NOT NULL);");
}

void DbVersionConverterTestTest::printErrors()
{
    for (const QString& err : converter->getErrors())
        qWarning() << err;
}

void DbVersionConverterTestTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    converter = new DbVersionConverter();
}

void DbVersionConverterTestTest::init()
{
    converter->reset();
}

void DbVersionConverterTestTest::cleanupTestCase()
{
    delete converter;
}

QTEST_APPLESS_MAIN(DbVersionConverterTestTest)

#include "tst_dbversionconvertertesttest.moc"
