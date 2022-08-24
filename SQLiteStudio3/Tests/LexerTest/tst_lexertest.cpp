#include "parser/lexer.h"
#include <QString>
#include <QtTest>

class LexerTest : public QObject
{
    Q_OBJECT

    public:
        LexerTest();

    private Q_SLOTS:
        void testStringCase1();
        void testStringCase2();
        void testFloat();
        void testHex1();
        void testHex2();
        void testBindParam1();
        void testBlobLiteral();
};

LexerTest::LexerTest()
{
}

void LexerTest::testStringCase1()
{
    QString sql = "INSERT INTO tab VALUES (1, 2, :val); /* test";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 20);
}

void LexerTest::testStringCase2()
{
    QString sql = "SELECT 1 = '1'";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QCOMPARE(tokens.size(), 7);
    QCOMPARE(tokens[2]->type, Token::INTEGER);
    QCOMPARE(tokens[6]->type, Token::STRING);
}

void LexerTest::testFloat()
{
    QString sql = "SELECT .2";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 3);
    QVERIFY(tokens[2]->type == Token::FLOAT);
}

void LexerTest::testHex1()
{
    QString sql = "SELECT 0x";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 3);
    QVERIFY(tokens[2]->type == Token::INVALID);
}

void LexerTest::testHex2()
{
    QString sql = "SELECT 0x5zzz";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 4);
    QVERIFY(tokens[2]->type == Token::INTEGER);
    QVERIFY(tokens[3]->type == Token::OTHER);
    QVERIFY(tokens[3]->value == "zzz");
}

void LexerTest::testBindParam1()
{
    QString sql = "SELECT * FROM test WHERE id = ?1 OR id = ?123 OR id = ? OR id = :id OR id = @id";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    TokenList bindTokens = tokens.filter(Token::BIND_PARAM);
    QVERIFY(bindTokens.size() == 5);
    QVERIFY(bindTokens[0]->value == "?1");
    QVERIFY(bindTokens[1]->value == "?123");
    QVERIFY(bindTokens[2]->value == "?");
    QVERIFY(bindTokens[3]->value == ":id");
    QVERIFY(bindTokens[4]->value == "@id");
}

void LexerTest::testBlobLiteral()
{
    QString sql = "SELECT X'010f0E'";

    Lexer lex;
    TokenList tokens = lex.tokenize(sql);
    QCOMPARE(tokens.size(), 3);
    QCOMPARE(tokens[2]->value, "X'010f0E'");
}

QTEST_APPLESS_MAIN(LexerTest)

#include "tst_lexertest.moc"
