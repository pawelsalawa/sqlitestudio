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
        void testFloat();
        void testHex1();
        void testHex2();
};

LexerTest::LexerTest()
{
}

void LexerTest::testStringCase1()
{
    QString sql = "INSERT INTO tab VALUES (1, 2, :val); /* test";

    Lexer lex(Dialect::Sqlite3);
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 20);
}

void LexerTest::testFloat()
{
    QString sql = "SELECT .2";

    Lexer lex(Dialect::Sqlite3);
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 3);
    QVERIFY(tokens[2]->type == Token::FLOAT);
}

void LexerTest::testHex1()
{
    QString sql = "SELECT 0x";

    Lexer lex(Dialect::Sqlite3);
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 3);
    QVERIFY(tokens[2]->type == Token::INVALID);
}

void LexerTest::testHex2()
{
    QString sql = "SELECT 0x5zzz";

    Lexer lex(Dialect::Sqlite3);
    TokenList tokens = lex.tokenize(sql);
    QVERIFY(tokens.size() == 4);
    QVERIFY(tokens[2]->type == Token::INTEGER);
    QVERIFY(tokens[3]->type == Token::OTHER);
    QVERIFY(tokens[3]->value == "zzz");
}

QTEST_APPLESS_MAIN(LexerTest)

#include "tst_lexertest.moc"
