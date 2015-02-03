#include "parser/parser.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "parser/parsererror.h"
#include <QString>
#include <QtTest>
#include <parser/ast/sqliteinsert.h>

class ParserTest : public QObject
{
        Q_OBJECT

    public:
        ParserTest();

    private:
        Parser* parser2 = nullptr;
        Parser* parser3 = nullptr;

    private Q_SLOTS:
        void testGetTableTokens();
        void testGetTableTokens2();
        void testGetDatabaseTokens();
        void testGetFullObjects();
        void testGetFullObjects2();
        void testUnfinishedSingleSourceWithTolerance();
        void testCommentEnding1();
        void testCommentEnding2();
        void testOper1();
        void testBig1();
        void testTableFk();
        void testDoubleQuotes();
        void testInsertError();
        void testExpr();
        void testCommentBeginMultiline();
        void testBetween();
        void testBigNum();
        void initTestCase();
        void cleanupTestCase();
};

ParserTest::ParserTest()
{
}

void ParserTest::testGetTableTokens()
{
    QString sql = "select someTable.* FROM someTable;";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    TokenList tokens = query->getContextTableTokens();
    QVERIFY(tokens.size() == 2);
    QVERIFY(tokens[0]->type == Token::OTHER);
    QVERIFY(tokens[1]->type == Token::OTHER);
}

void ParserTest::testGetTableTokens2()
{
    QString sql = "select db.tab.col FROM someTable;";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    TokenList tokens = query->getContextTableTokens();
    QVERIFY(tokens.size() == 2);
    QVERIFY(tokens[0]->type == Token::OTHER);
    QVERIFY(tokens[1]->type == Token::OTHER);
}

void ParserTest::testGetDatabaseTokens()
{
    QString sql = "select * FROM someDb.[table];";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    TokenList tokens = query->getContextTableTokens();
    QVERIFY(tokens.size() == 1);
    QVERIFY(tokens[0]->type == Token::OTHER);
}

void ParserTest::testGetFullObjects()
{
    QString sql = "select col FROM someDb.[table];";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    QList<SqliteStatement::FullObject> fullObjects = query->getContextFullObjects();
    QVERIFY(fullObjects.size() == 2);

    foreach (const SqliteStatement::FullObject& fullObj, fullObjects)
    {
        switch (fullObj.type)
        {
            case SqliteStatement::FullObject::TABLE:
                QVERIFY(fullObj.database && fullObj.database->value == "someDb");
                QVERIFY(fullObj.object && fullObj.object->value == "[table]");
                break;
            case SqliteStatement::FullObject::DATABASE:
                QVERIFY(fullObj.database && fullObj.database->value == "someDb");
                break;
            default:
                QFAIL("Unexpected FullObject type.");
        }
    }
}

void ParserTest::testGetFullObjects2()
{
    QString sql = "select col, tab2.*, abcDb.abcTab.abcCol FROM someDb.[table];";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    QList<SqliteStatement::FullObject> fullObjects = query->getContextFullObjects();
    QVERIFY(fullObjects.size() == 5);

    foreach (const SqliteStatement::FullObject& fullObj, fullObjects)
    {
        switch (fullObj.type)
        {
            case SqliteStatement::FullObject::TABLE:
            {
                QVERIFY(fullObj.object);
                if (fullObj.database && fullObj.database->value == "someDb")
                    QVERIFY(fullObj.object->value == "[table]");
                else if (fullObj.database && fullObj.database->value == "abcDb")
                    QVERIFY(fullObj.object->value == "abcTab");
                else if (!fullObj.database)
                    QVERIFY(fullObj.object->value == "tab2");
                else
                    QFAIL("Invalid TABLE full object.");

                break;
            }
            case SqliteStatement::FullObject::DATABASE:
            {
                if (fullObj.database)
                    QVERIFY(fullObj.database->value == "someDb" || fullObj.database->value == "abcDb");
                else
                    QFAIL("Invalid DATABASE full object.");

                break;
            }
            default:
                QFAIL("Unexpected FullObject type.");
        }
    }
}

void ParserTest::testUnfinishedSingleSourceWithTolerance()
{
    QString sql = "SELECT * FROM test.;";
    bool res = parser3->parse(sql, true);
    QVERIFY(res);
}

void ParserTest::testCommentEnding1()
{
    QString sql = "select 1 --aaa";
    bool res = parser3->parse(sql);
    QVERIFY(res);
}

void ParserTest::testCommentEnding2()
{
    QString sql = "select 1 /*aaa";
    bool res = parser3->parse(sql);
    QVERIFY(res);
}

void ParserTest::testOper1()
{
    QString sql = "SELECT dfgd<=2";
    TokenList tokens = Lexer::tokenize(sql, Dialect::Sqlite3);
    QVERIFY(tokens[2]->value == "dfgd");
    QVERIFY(tokens[3]->value == "<=");
    QVERIFY(tokens[4]->value == "2");
}

void ParserTest::testBig1()
{
    QString sql = "select "
        "'<DemandReportForm>'|| "
        "'<TerrOrganEmail/>'|| "
        "'<TerrOrganName></TerrOrganName>'|| "
        "'<TerrOrganAddress></TerrOrganAddress>'|| "
        "'<TerrOrganPhoneAndFax></TerrOrganPhoneAndFax>'|| "
        "'<TerrOrganPhone></TerrOrganPhone>'|| "
        "'<TerrOrganFax></TerrOrganFax>'|| "
        "'<P1>Ð�TH;°Ñ禅TH;°Ð»Ñ즅TH;½Ð¸Ðº Ñ㦅TH;¿ÑঅTH;°Ð²Ð»ÐµÐ½Ð¸Ñ怜t;/P1>'|| "
        "'<P2></P2>'|| "
        "'<CreateDate>'||strftime('%d.%m.%Y',d.demdate)||'</CreateDate>'|| "
        "'<NumberDemand>4642'||substr('000000'||d.id, -6, 6)||'</NumberDemand>'|| "
        "'<ControlDepartmentName></ControlDepartmentName>'|| "
        "'<InsurerName>'||ins.fullname||'</InsurerName>'|| "
        "'<InsurerShortName>'||ins.shortname||'</InsurerShortName>'|| "
        "'<RegNumPFR>0'||ins.regnum||'</RegNumPFR>'|| "
        "'<HeadName/>'|| "
        "'<INN>'||ins.inn||'</INN>'|| "
        "'<KPP>'||ins.kpp||'</KPP>'|| "
        "'<InsurerAddress>'||ins.addr||'</InsurerAddress>'|| "
        "'<InsurerPostAddress>'||ins.postaddr||'</InsurerPostAddress>'|| "
        "'<TotalArrearsSum>'||round(dem_s+dem_n+dem_f+dem_t,2)||'</TotalArrearsSum>'|| "
        "'<ControlDate>'||strftime('%d.%m.%Y',d.demdate,'+15 day')||'</ControlDate>'|| "
        "'<TotalContributionPFR>0,00</TotalContributionPFR>'|| "
        "'<ContributionPFRInsurer>0,00</ContributionPFRInsurer>'|| "
        "'<ContributionPFRInsurerKBK/>'|| "
        "'<ContributionPFRHoarder>0,00</ContributionPFRHoarder>'|| "
        "'<ContributionPFRHoarderKBK/>'|| "
        "'<TotalContributionOMS>0,00</TotalContributionOMS>'|| "
        "'<ContributionFFOMS>0,00</ContributionFFOMS>'|| "
        "'<ContributionFFOMSKBK/>'|| "
        "'<ContributionTFOMS>0,00</ContributionTFOMS>'|| "
        "'<ContributionTFOMSKBK/>'|| "
        "'<TotalContributionTFOMS>0,00</TotalContributionTFOMS>'|| "
        "'<TotalContributionFFOMS>0,00</TotalContributionFFOMS>'|| "
        "'<TotalFine>'||round(dem_s+dem_n+dem_f+dem_t,2)||'</TotalFine>'|| "
        "'<TotalPFRFine>'||round(dem_s+dem_n,2)||'</TotalPFRFine>'|| "
        "'<TotalPFRPenalty>0,00</TotalPFRPenalty>'|| "
        "'<FinePFRInsurer>'||round(dem_s,2)||'</FinePFRInsurer>'|| "
        "'<FinePFRInsurerKBK>'||case when d.dem_s>0 then '(ÐꦅTH;ᦅTH;ꠧ'||kbk.kbk_s||')' else '' end || '</FinePFRInsurerKBK>'|| "
        "'<FinePFRHoarder>'||round(dem_n,2)||'</FinePFRHoarder>'|| "
        "'<FinePFRHoarderKBK>'||case when d.dem_n>0 then '(ÐꦅTH;ᦅTH;ꠧ'||kbk.kbk_n||')' else '' end||'</FinePFRHoarderKBK>'|| "
        "'<FineFFOMS>'||round(dem_f,2)||'</FineFFOMS>'|| "
        "'<FineFFOMSKBK>'||case when d.dem_f>0 then '(ÐꦅTH;ᦅTH;ꠧ'||kbk.kbk_f||')' else '' end||'</FineFFOMSKBK>'|| "
        "'<FineTFOMS>'||round(dem_t,2)||'</FineTFOMS>'|| "
        "'<FineTFOMSKBK>'||case when d.dem_t>0 then '(ÐꦅTH;ᦅTH;ꠧ'||kbk.kbk_t||')' else '' end||'</FineTFOMSKBK>'|| "
        "'<PenaltyPFRSum>0,00</PenaltyPFRSum>'|| "
        "'<PenaltyPFRWithDetails>0,00 0,00 0,00 0,00</PenaltyPFRWithDetails>'|| "
        "'<PenaltyPFRRub>Ñ঎tilde;㦅TH;±.;</PenaltyPFRRub>'|| "
        "'<PenaltyPFR>0,00</PenaltyPFR>'|| "
        "'<PenaltyPFRKBK/>'|| "
        "'<PenaltyPFRInsurance>0,00</PenaltyPFRInsurance>'|| "
        "'<PenaltyPFRInsuranceKBK/>'|| "
        "'<PenaltyPFRHoarder>0,00</PenaltyPFRHoarder>'|| "
        "'<PenaltyPFRHoarderKBK/>'|| "
        "'<PenaltyFFOMS>0,00</PenaltyFFOMS>'|| "
        "'<PenaltyFFOMSKBK/>'|| "
        "'<PenaltyTFOMS>0,00</PenaltyTFOMS>'|| "
        "'<PenaltyTFOMSKBK/>'|| "
        "'<ClarifiedDemandDate/>'|| "
        "'<ClarifiedDemandNumber/>'|| "
        "'<ArrearsDate>'||strftime('%d.%m.%Y',d.demdate)||'</ArrearsDate>'|| "
        "'<ContributionPFR>0</ContributionPFR>'|| "
        "'<ContributionPFRKBK/>'|| "
        "'<FinePFR>0</FinePFR>'|| "
        "'<FinePFRKBK/>'|| "
        "'<ExecutorName>'||ins.Specname||'</ExecutorName>'|| "
        "'<ExecutorPhone></ExecutorPhone>'|| "
        "'</DemandReportForm>' "
        "from demands d "
        "left join ins on ins.regnum=d.regnum "
        "left join kbk on kbk.cat=ins.Cat "
        "limit 1";

    bool res = parser3->parse(sql);
    if (!res)
    {
        qWarning() << parser3->getErrorString();
        ParserError* error = parser3->getErrors().first();
        qDebug() << "Error starts at:" << sql.mid(error->getFrom());
    }

    QVERIFY(res);
}

void ParserTest::testTableFk()
{
    QString sql = "CREATE TABLE test (id INTEGER, FOREIGN KEY (id) REFERENCES test2 (id2));";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    SqliteCreateTablePtr creatrTable = query.dynamicCast<SqliteCreateTable>();
    QVERIFY(creatrTable->constraints.size() == 1);
    QVERIFY(creatrTable->constraints.first()->indexedColumns.size() == 1);
    QVERIFY(creatrTable->constraints.first()->indexedColumns.first()->name == "id");
    QVERIFY(creatrTable->constraints.first()->type == SqliteCreateTable::Constraint::FOREIGN_KEY);
    QVERIFY(creatrTable->constraints.first()->foreignKey != nullptr);
    QVERIFY(creatrTable->constraints.first()->foreignKey->foreignTable == "test2");
    QVERIFY(creatrTable->constraints.first()->foreignKey->indexedColumns.size() == 1);
    QVERIFY(creatrTable->constraints.first()->foreignKey->indexedColumns.first()->name == "id2");
}

void ParserTest::testDoubleQuotes()
{
    QString sql = "select \"1\"";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getQueries().size() > 0);

    SqliteQueryPtr query = parser3->getQueries().first();
    QVERIFY(query);

    SqliteSelectPtr select = query.dynamicCast<SqliteSelect>();
    QVERIFY(select);
    QVERIFY(select->coreSelects.size() > 0);
    QVERIFY(select->coreSelects.first()->resultColumns.size() > 0);

    SqliteSelect::Core::ResultColumn* rc = select->coreSelects.first()->resultColumns.first();
    SqliteExpr* e = rc->expr;
    QVERIFY(e);

    QVERIFY(e->mode == SqliteExpr::Mode::ID);
    QVERIFY(e->possibleDoubleQuotedString);
}

void ParserTest::testInsertError()
{
    QString sql = "INSERT INTO test ";
    bool res = parser3->parse(sql);
    QVERIFY(!res);
    QVERIFY(parser3->getErrors().size() == 1);
}

void ParserTest::testExpr()
{
    QString sql = "CAST (CASE WHEN port REGEXP '^[A-Z]' THEN substr(port, 2) ELSE port END AS INT) AS port";
    SqliteExpr* expr = parser3->parseExpr(sql);
    QVERIFY(expr);
}

void ParserTest::testCommentBeginMultiline()
{
    QString sql = "/*";
    TokenList tokens = Lexer::tokenize(sql, Dialect::Sqlite3);
    QVERIFY(tokens.size() == 1);
    QVERIFY(tokens[0]->type == Token::COMMENT);
}

void ParserTest::testBetween()
{
    QString sql = "SELECT * FROM test WHERE a BETWEEN 1 and 2";
    bool res = parser3->parse(sql);
    QVERIFY(res);
}

void ParserTest::testBigNum()
{
    QString sql = "SELECT ( col - 73016000000 ) FROM tab";
    bool res = parser3->parse(sql);
    QVERIFY(res);
}

void ParserTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    parser2 = new Parser(Dialect::Sqlite2);
    parser3 = new Parser(Dialect::Sqlite3);
}

void ParserTest::cleanupTestCase()
{
    delete parser2;
    delete parser3;
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
