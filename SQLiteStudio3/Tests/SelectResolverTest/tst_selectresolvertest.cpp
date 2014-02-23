#include "selectresolver.h"
#include "db/db.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/parser.h"
#include "dbsqlite3mock.h"
#include <QString>
#include <QtTest>
#include <QSet>
#include <parser/parser.h>

class SelectResolverTest : public QObject
{
        Q_OBJECT

    public:
        SelectResolverTest();

    private:
        Db* db = nullptr;

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testTableHash();
        void testColumnHash();
        void test1();
};

SelectResolverTest::SelectResolverTest()
{
}

void SelectResolverTest::testTableHash()
{
    QSet<SelectResolver::Table> tables;

    SelectResolver::Table t1;
    t1.database = "d1";
    t1.table = "t1";
    t1.alias = "a1";
    tables << t1;

    // different alias and database
    SelectResolver::Table t2;
    t2.database = "d2";
    t2.table = "t1";
    t2.alias = QString::null;
    tables << t2;

    // different database
    SelectResolver::Table t3;
    t3.database = "d2";
    t3.table = "t1";
    t3.alias = "a1";
    tables << t3;

    // same as t3
    SelectResolver::Table t4;
    t4.database = "d2";
    t4.table = "t1";
    t4.alias = "a1";
    tables << t4;

    // all null
    SelectResolver::Table t5;
    tables << t5;

    // same as t5
    SelectResolver::Table t6;
    tables << t6;

    // similar to t1, but different database
    SelectResolver::Table t7;
    t7.database = "x";
    t7.table = "t1";
    t7.alias = "a1";
    tables << t7;

    // similar to t1, but different table
    SelectResolver::Table t8;
    t8.database = "d1";
    t8.table = "x";
    t8.alias = "a1";
    tables << t8;

    // similar to t1, but different alias
    SelectResolver::Table t9;
    t9.database = "d1";
    t9.table = "t1";
    t9.alias = "x";
    tables << t9;

    QVERIFY(tables.size() == 7);
}

void SelectResolverTest::testColumnHash()
{
    QSet<SelectResolver::Column> columns;

    SelectResolver::Column c1;
    c1.database = "d1";
    c1.table = "t1";
    c1.column = "c1";
    c1.alias = "a1";
    c1.tableAlias = "ta1";
    c1.displayName = "d1";
    c1.type = SelectResolver::Column::COLUMN;
    columns << c1;

    // This should be treated as equal to c1.
    SelectResolver::Column c2;
    c2.database = "d1";
    c2.table = "t1";
    c2.column = "c1";
    c2.alias = "x";
    c2.tableAlias = "ta1";
    c2.displayName = "x";
    c2.type = SelectResolver::Column::OTHER;
    columns << c2;

    // Different database
    SelectResolver::Column c3;
    c3.database = "x";
    c3.table = "t1";
    c3.column = "c1";
    c3.alias = "x";
    c3.tableAlias = "ta1";
    c3.displayName = "x";
    c3.type = SelectResolver::Column::OTHER;
    columns << c3;

    // Different table
    SelectResolver::Column c4;
    c4.database = "d1";
    c4.table = "x";
    c4.column = "c1";
    c4.alias = "x";
    c4.tableAlias = "ta1";
    c4.displayName = "x";
    c4.type = SelectResolver::Column::OTHER;
    columns << c4;

    // Different column
    SelectResolver::Column c5;
    c5.database = "d1";
    c5.table = "t1";
    c5.column = "x";
    c5.alias = "x";
    c5.tableAlias = "ta1";
    c5.displayName = "x";
    c5.type = SelectResolver::Column::OTHER;
    columns << c5;

    // Different table alias
    SelectResolver::Column c6;
    c6.database = "d1";
    c6.table = "t1";
    c6.column = "c1";
    c6.alias = "x";
    c6.tableAlias = "x";
    c6.displayName = "x";
    c6.type = SelectResolver::Column::OTHER;
    columns << c6;

    QVERIFY(columns.size() == 5);
}

void SelectResolverTest::test1()
{
    QString sql = "SELECT * FROM (SELECT count(col1), col2 FROM test)";
    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column> > columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QList<SelectResolver::Column> coreColumns = columns.first();
    QVERIFY(coreColumns[0].type == SelectResolver::Column::OTHER);
    QVERIFY(coreColumns[1].type == SelectResolver::Column::COLUMN);
    QVERIFY(coreColumns[1].table == "test");
    QVERIFY(coreColumns[1].column == "col2");
}

void SelectResolverTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();

    db = new DbSqlite3Mock("testdb");
    db->open();
    db->exec("CREATE TABLE test (col1, col2, col3);");
}

void SelectResolverTest::cleanupTestCase()
{
    db->close();
    delete db;
    db = nullptr;
}

QTEST_APPLESS_MAIN(SelectResolverTest)

#include "tst_selectresolvertest.moc"
