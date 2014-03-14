#include "completionhelper.h"
#include "expectedtoken.h"
#include "dbsqlite3mock.h"
#include "parser/lexer.h"
#include "parser/token.h"
#include "parser/keywords.h"
#include "sqlitestudio.h"
#include "mocks.h"
#include <QString>
#include <QtTest>
#include <QDebug>


class CompletionHelperTest : public QObject
{
    Q_OBJECT

    public:
        CompletionHelperTest();

    private:
        QList<ExpectedToken*> getEntryList(QList<ExpectedTokenPtr> tokens);
        QSet<ExpectedToken::Type> getTypeList(QList<ExpectedTokenPtr> tokens);

        Db* db = nullptr;

        bool contains(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type);
        bool contains(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type,
                      const QString& value);
        bool contains(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type,
                      const QString& value, const QString& prefix);
        bool contains(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type,
                      const QString &value, const QString &prefix, const QString &contextInfo);

        int find(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type);
        int find(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type,
                 const QString& value);
        int find(const QList<ExpectedTokenPtr>& tokens, ExpectedToken::Type type,
                 const QString& value, const QString& prefix);
        int find(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type,
                 const QString &value, const QString &prefix, const QString &contextInfo);

    private Q_SLOTS:
        void testResCol1();
        void testFrom1();
        void testFrom2();
        void testResCol2();
        void testResCol3();
        void testResCol4();
        void testResCol5();
        void testResCol6();
        void testFromKw();
        void testUpdateTable();
        void testUpdateCols1();
        void initTestCase();
        void cleanupTestCase();
};

CompletionHelperTest::CompletionHelperTest()
{
}

QList<ExpectedToken *> CompletionHelperTest::getEntryList(QList<ExpectedTokenPtr> tokens)
{
    QList<ExpectedToken*> entries;
    foreach (ExpectedTokenPtr expectedToken, tokens)
        entries += expectedToken.data();

    return entries;
}

QSet<ExpectedToken::Type> CompletionHelperTest::getTypeList(QList<ExpectedTokenPtr> tokens)
{
    QSet<ExpectedToken::Type> entries;
    foreach (ExpectedTokenPtr expectedToken, tokens)
        entries += expectedToken->type;

    return entries;
}

bool CompletionHelperTest::contains(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type)
{
    return find(tokens, type) > -1;
}

bool CompletionHelperTest::contains(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value)
{
    return find(tokens, type, value) > -1;
}

bool CompletionHelperTest::contains(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value, const QString &prefix)
{
    return find(tokens, type, value, prefix) > -1;
}

bool CompletionHelperTest::contains(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value, const QString &prefix, const QString &contextInfo)
{
    return find(tokens, type, value, prefix, contextInfo) > -1;
}

int CompletionHelperTest::find(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type)
{
    int i = 0;
    foreach (ExpectedTokenPtr token, tokens)
    {
        if (token->type == type)
            return i;

        i++;
    }
    return -1;
}

int CompletionHelperTest::find(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value)
{
    int i = -1;
    foreach (ExpectedTokenPtr token, tokens)
    {
        i++;
        if (token->type != type)
            continue;

        if (token->value == value)
            return i;
    }
    return -1;
}

int CompletionHelperTest::find(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value, const QString &prefix)
{
    int i = -1;
    foreach (ExpectedTokenPtr token, tokens)
    {
        i++;
        if (token->type != type)
            continue;

        if (token->value != value)
            continue;

        if (token->prefix == prefix)
            return i;
    }
    return -1;
}

int CompletionHelperTest::find(const QList<ExpectedTokenPtr> &tokens, ExpectedToken::Type type, const QString &value, const QString &prefix, const QString &contextInfo)
{
    int i = -1;
    foreach (ExpectedTokenPtr token, tokens)
    {
        i++;
        if (token->type != type)
            continue;

        if (token->value != value)
            continue;

        if (token->prefix != prefix)
            continue;

        if (token->contextInfo == contextInfo)
            return i;
    }
    return -1;
}

void CompletionHelperTest::testFrom1()
{
    QString sql = "select * FROM ";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    QVERIFY(contains(tokens, ExpectedToken::TABLE, "test"));
    QVERIFY(contains(tokens, ExpectedToken::TABLE, "sqlite_master"));
    QVERIFY(contains(tokens, ExpectedToken::TABLE, "sqlite_temp_master"));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE, "main"));
}

void CompletionHelperTest::testFrom2()
{
    QString sql = "select id from abc, ";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

//    QList<ExpectedToken*> entries = getEntryList(tokens);

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(!contains(tokens, ExpectedToken::FUNCTION));
    QVERIFY(!contains(tokens, ExpectedToken::COLUMN));

    // Because abc was already used, the order should be:
    // sqlite_master
    // sqlite_temp_master
    // test
    // abc
    QVERIFY(find(tokens, ExpectedToken::TABLE, "sqlite_master") == 0);
    QVERIFY(find(tokens, ExpectedToken::TABLE, "sqlite_temp_master") == 1);
    QVERIFY(find(tokens, ExpectedToken::TABLE, "test") == 2);
    QVERIFY(find(tokens, ExpectedToken::TABLE, "abc") == 3);
}

void CompletionHelperTest::testResCol1()
{
    QString sql = "select main.test.";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    QVERIFY(!contains(tokens, ExpectedToken::COLUMN, "name"));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "id"));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "val"));
}

void CompletionHelperTest::testResCol2()
{
    QString sql = "select ";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    //QList<ExpectedToken*> entries = getEntryList(tokens);

    // Exclude JOIN keywords and FK MATCH keywords
    QVERIFY(!contains(tokens, ExpectedToken::KEYWORD, "NATURAL"));
    QVERIFY(!contains(tokens, ExpectedToken::KEYWORD, "CROSS"));
    QVERIFY(!contains(tokens, ExpectedToken::KEYWORD, "SIMPLE"));
    QVERIFY(!contains(tokens, ExpectedToken::KEYWORD, "FULL"));

    // Exclude OTHER as there is at least one CTX token available
    QVERIFY(!contains(tokens, ExpectedToken::OTHER));

    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(contains(tokens, ExpectedToken::FUNCTION));
    QVERIFY(contains(tokens, ExpectedToken::TABLE, "sqlite_master"));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "name", QString::null));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "id", QString::null));
}

void CompletionHelperTest::testResCol3()
{
    QString sql = "select  from test";
    CompletionHelper helper(sql, 7, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    //QList<ExpectedToken*> entries = getEntryList(tokens);

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(contains(tokens, ExpectedToken::FUNCTION));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "id", QString::null));
    QVERIFY(contains(tokens, ExpectedToken::COLUMN, "val", QString::null));

    // Order should be:
    // id - test (no prefix value)
    // val - test (default, no prefix value)
    // val2 - test (default, no prefix value)
    // id - abc (no prefix, we didn't mention other table in from clause)
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "test") == 0);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val", QString::null, "test") == 1);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val2", QString::null, "test") == 2);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "abc") == 3);
}

void CompletionHelperTest::testResCol4()
{
    QString sql = "select test.id, from test";
    CompletionHelper helper(sql, 15, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    //QList<ExpectedToken*> entries = getEntryList(tokens);

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(contains(tokens, ExpectedToken::FUNCTION));

    // Because test.id was already used, the order should be:
    // val - test (default, no prefix value)
    // val2 - test (default, no prefix value)
    // id - test (no prefix, only one table in FROM)
    // id - abc (no prefix, only one table in FROM)
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val", QString::null, "test") == 0);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val2", QString::null, "test") == 1);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "test") == 2);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "abc") == 3);
}

void CompletionHelperTest::testResCol5()
{
    QString sql = "select test.id, val from test";
    CompletionHelper helper(sql, 15, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    //QList<ExpectedToken*> entries = getEntryList(tokens);

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(contains(tokens, ExpectedToken::FUNCTION));

    // Because test.id and val were already used, the order should be:
    // val2 - test (default, no prefix value)
    // id - test
    // val - test (default, no prefix value)
    // id - abc (no prefix, only one table in FROM)
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val2") == 0);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "test") == 1);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val") == 2);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "abc") == 3);
}

void CompletionHelperTest::testResCol6()
{
    QString sql = "select (select  from sqlite_master as S, sqlite_master as S2) from test as A";
    CompletionHelper helper(sql, 15, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    //QList<ExpectedToken*> entries = getEntryList(tokens);

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
    QVERIFY(contains(tokens, ExpectedToken::DATABASE));
    QVERIFY(contains(tokens, ExpectedToken::FUNCTION));

    // Desired order:
    // S.name - sqlite_master S   (can be second or first)
    // S.name - sqlite_master S2  (can be second or first)
    // ...
    // id - test (default, no prefix)
    // ...
    // id - abc (no prefix, only one table with this column in FROM)
    // ...
    int s_sqlite_master_name = find(tokens, ExpectedToken::COLUMN, "name", "S", "sqlite_master");
    int s2_sqlite_master_name = find(tokens, ExpectedToken::COLUMN, "name", "S2", "sqlite_master");
    int test_id = find(tokens, ExpectedToken::COLUMN, "id", "A", "test");
    int abc_id = find(tokens, ExpectedToken::COLUMN, "id", QString::null, "abc");
    QVERIFY(s_sqlite_master_name <= 1);
    QVERIFY(s2_sqlite_master_name <= 1);
    QVERIFY(test_id > s2_sqlite_master_name);
    QVERIFY(abc_id > test_id);
}

void CompletionHelperTest::testFromKw()
{
    QString sql = "select * FR";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    QVERIFY(contains(tokens, ExpectedToken::KEYWORD, "FROM"));
    QVERIFY(!contains(tokens, ExpectedToken::COLUMN));
    QVERIFY(tokens.size() == 1);
}

void CompletionHelperTest::testUpdateTable()
{
    QString sql = "update ";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    QVERIFY(contains(tokens, ExpectedToken::TABLE));
}

void CompletionHelperTest::testUpdateCols1()
{
    QString sql = "update test set id = 5, ";
    CompletionHelper helper(sql, db);
    QList<ExpectedTokenPtr> tokens = helper.getExpectedTokens().filtered();

    // TODO if table is provided and there is at least one column in proposal for it, then skip columns from other tables.
    // TODO Make the context more precise - distinguish between left side column from right side expression

    QVERIFY(find(tokens, ExpectedToken::COLUMN, "id", QString::null, "test") == 0);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val", QString::null, "test") == 1);
    QVERIFY(find(tokens, ExpectedToken::COLUMN, "val2", QString::null, "test") == 2);
    //QVERIFY(!contains(tokens, ExpectedToken::COLUMN, "id", QString::null, "abc")); // TODO
}

void CompletionHelperTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    CompletionHelper::init();
    initMocks();

    db = new DbSqlite3Mock("testdb");
    db->open();
    db->exec("CREATE TABLE test (id int, val text, val2 text);");
    db->exec("CREATE TABLE abc (id int, xyz text);");
}

void CompletionHelperTest::cleanupTestCase()
{
    db->close();
    delete db;
    db = nullptr;
    deleteMockRepo();
}

QTEST_APPLESS_MAIN(CompletionHelperTest)

#include "tst_completionhelpertest.moc"
