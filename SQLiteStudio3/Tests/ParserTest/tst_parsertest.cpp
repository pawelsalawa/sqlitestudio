#include "parser/parser.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "parser/parsererror.h"
#include "common/utils_sql.h"
#include "parser/ast/sqlitewindowdefinition.h"
#include "parser/ast/sqlitefilterover.h"
#include <QString>
#include <QtTest>

class ParserTest : public QObject
{
        Q_OBJECT

    public:
        ParserTest();

    private:
        Parser* parser3 = nullptr;
        void verifyWindowClause(const QString& sql, SqliteSelectPtr& select, bool& ok);

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();

        void test();
        void testString();
        void testScientificNumber();
        void testUniqConflict();
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
        void testSelectWith();
        void testInsertWithDoubleQuoteValues();
        void testInsertIncompleteOnColumnName();
        void testParseAndRebuildAlias();
        void testRebuildTokensUpdate();
        void testRebuildTokensInsertUpsert();
        void testGetColumnTokensFromInsertUpsert();
        void testGeneratedColumn();
        void testWindowClause();
        void testWindowKwAsColumn();
        void testFilterClause();
        void testFilterAsId();
        void testUpdateFrom();
        void testStringAsTableId();
        void testJsonPtrOp();
        void testUnfinishedSelectWithAliasForCompleter();
        void testUnfinishedSelectWithAliasStrict();
        void testBlobLiteral();
        void testBigDec();
        void testQuotedFunction();
        void testIndexedSelect();
};

ParserTest::ParserTest()
{
}

void ParserTest::test()
{
    QString sql = "CREATE TRIGGER param_insert_chk_enum "
            "BEFORE INSERT "
            "ON param "
            "WHEN new.type = 'enum' AND "
            "new.defval IS NOT NULL AND "
            "new.defval != '' "
            "BEGIN "
            "SELECT RAISE(FAIL, 'param_insert_chk_enum failed') "
            "WHERE NOT EXISTS ( "
            "SELECT val "
            "FROM valset "
            "WHERE param_id = new.param_id AND "
            "val = new.defval "
            "); "
            "END;";

    parser3->parse(sql);
    QVERIFY(parser3->getErrors().size() == 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    TokenList tokens = query->getContextTableTokens();
}

void ParserTest::testString()
{
    QString sql = "SELECT 1 = '1';";

    parser3->parse(sql);
    QCOMPARE(parser3->getErrors().size(), 0);

    SqliteQueryPtr query = parser3->getQueries()[0];
    query->rebuildTokens();

    QCOMPARE(query->tokens.detokenize(), sql);
    QCOMPARE(query->tokens.size(), 8);
    QCOMPARE(query->tokens[2]->type, Token::Type::INTEGER);
    QCOMPARE(query->tokens[6]->type, Token::Type::STRING);
}

void ParserTest::testScientificNumber()
{
    QString sql = "SELECT 1e100;";
    TokenList tokens = Lexer::tokenize(sql);

    QVERIFY(tokens.size() == 4);
    QVERIFY(tokens[2]->type == Token::Type::FLOAT);
}

void ParserTest::testGetTableTokens()
{
    QString sql = "select someTable.* FROM someTable;";

    parser3->parse(sql);
    QCOMPARE(parser3->getErrors().size(), 0);

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
    QCOMPARE(parser3->getErrors().size(), 0);

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

    for (const SqliteStatement::FullObject& fullObj : fullObjects)
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

    for (const SqliteStatement::FullObject& fullObj : fullObjects)
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
    TokenList tokens = Lexer::tokenize(sql);
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
    TokenList tokens = Lexer::tokenize(sql);
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

void ParserTest::testUniqConflict()
{
    QString sql = "CREATE TABLE test (x UNIQUE ON CONFLICT FAIL);";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    SqliteQueryPtr q = parser3->getQueries().first();
    TokenList tokens = q->tokens;
    QVERIFY(tokens[16]->type == Token::Type::PAR_RIGHT);
}

void ParserTest::testSelectWith()
{
    QString sql = "WITH m (c1, c2) AS (VALUES (1, 'a'), (2, 'b')) SELECT * FROM m;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    const SqliteQueryPtr query = parser3->getQueries().first();
    query->rebuildTokens();
    QString detokenized = query->detokenize().replace(" ", "");
    QVERIFY(sql.replace(" ", "") == detokenized);
}

void ParserTest::testInsertWithDoubleQuoteValues()
{
    QString sql = "REPLACE INTO _Variables (Name, TextValue) VALUES (\"varNowTime\", strftime(\"%Y-%m-%dT%H:%M:%S\", \"now\", \"localtime\"));";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    const SqliteInsertPtr insert = parser3->getQueries().first().dynamicCast<SqliteInsert>();
    insert->rebuildTokens();
    QString detokenized = insert->detokenize().replace(" ", "");
    QVERIFY(sql.replace(" ", "") == detokenized);
}

void ParserTest::testInsertIncompleteOnColumnName()
{
    QString sql = "INSERT INTO tabName (";
    bool res = parser3->parse(sql, true);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    const SqliteInsertPtr insert = parser3->getQueries().first().dynamicCast<SqliteInsert>();
    QVERIFY(insert->table == "tabName");
    QVERIFY(insert->columnNames.isEmpty());
}

void ParserTest::testParseAndRebuildAlias()
{
    QString sql = "SELECT x AS [\"abc\".\"def\"];";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteQueryPtr query = parser3->getQueries().first();
    query->rebuildTokens();
    QString newSql = query->detokenize();
    QVERIFY(sql == newSql);
}

void ParserTest::testRebuildTokensUpdate()
{
    QString sql = "UPDATE tab SET col1 = 1, (col2, col3) = 2 WHERE x = 3;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteUpdatePtr update = parser3->getQueries().first().dynamicCast<SqliteUpdate>();
    QVERIFY(update->keyValueMap.size() == 2);
    QVERIFY(update->keyValueMap[1].first.metaType() == QMetaType::fromType<QStringList>());
    QStringList set2List = update->keyValueMap[1].first.toStringList();
    QVERIFY(set2List[0] == "col2");
    QVERIFY(set2List[1] == "col3");
    QVERIFY(update->where);
    QVERIFY(!update->table.isNull());
    QVERIFY(update->where);
    update->rebuildTokens();
    QVERIFY(update->tokens.detokenize() == sql);
}

void ParserTest::testRebuildTokensInsertUpsert()
{
    QString sql = "INSERT INTO tab (a1, a2) VALUES (123, 456) ON CONFLICT (b1, b2, b3) DO UPDATE SET col1 = 1, (col2, col3) = 2 WHERE x = 3;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteInsertPtr insert = parser3->getQueries().first().dynamicCast<SqliteInsert>();
    QVERIFY(insert->upsert);

    insert->rebuildTokens();
    QVERIFY(insert->tokens.detokenize() == sql);
}

void ParserTest::testGetColumnTokensFromInsertUpsert()
{
    QString sql = "INSERT INTO tab (a1, a2) VALUES (123, 456) ON CONFLICT (b1, b2, b3) DO UPDATE SET col1 = 1, (col2, col3) = 2 WHERE x = 3;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteInsertPtr insert = parser3->getQueries().first().dynamicCast<SqliteInsert>();
    QVERIFY(insert->upsert);

    TokenList tk = insert->getContextColumnTokens();
    sSort(tk, [](const TokenPtr& t1, const TokenPtr& t2) {return t1->start < t2->start;});
    QVERIFY(tk.toValueList().join(" ") == "a1 a2 b1 b2 b3 col1 col2 col3 x");
}

void ParserTest::testGeneratedColumn()
{
    QString sql = "create table t2 (a INTEGER PRIMARY KEY AUTOINCREMENT, b INTEGER GENERATED ALWAYS AS (a+2) STORED);";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteCreateTablePtr create = parser3->getQueries().first().dynamicCast<SqliteCreateTable>();
    QVERIFY(create->columns.size() == 2);
    QVERIFY(create->columns[1]->constraints.size() == 1);
    QVERIFY(create->columns[1]->constraints[0]->type == SqliteCreateTable::Column::Constraint::GENERATED);
    QVERIFY(create->columns[1]->constraints[0]->generatedKw == true);
    QVERIFY(create->columns[1]->constraints[0]->generatedType == SqliteCreateTable::Column::Constraint::GeneratedType::STORED);
    QVERIFY(create->columns[1]->constraints[0]->expr);
}

void ParserTest::testWindowClause()
{
    QString sql = "SELECT x, y, row_number() OVER win1, rank() OVER win2 "
                  "  FROM t0 "
                  "WINDOW win1 AS (ORDER BY y RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW),"
                  "       win2 AS (PARTITION BY y ORDER BY x)"
                  " ORDER BY x;";

    SqliteSelectPtr select;
    bool ok = false;
    verifyWindowClause(sql, select, ok);
    if (!ok)
        return;

    qInfo() << "first run PASS, runing second time, after detokenizing";
    sql = select->detokenize();
    verifyWindowClause(sql, select, ok);
}

void ParserTest::testWindowKwAsColumn()
{
    QString sql = "SELECT window FROM test_table;";
    //parser3->setLemonDebug(true);
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());
}

void ParserTest::verifyWindowClause(const QString& sql, SqliteSelectPtr& select, bool& ok)
{
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    select = parser3->getQueries().first().dynamicCast<SqliteSelect>();
    QCOMPARE(select->coreSelects.size(), 1);
    SqliteSelect::Core* core = select->coreSelects[0];

    // Result Columns
    QCOMPARE(core->resultColumns.size(), 4);

    QVERIFY(core->resultColumns[0]->expr);
    QCOMPARE(core->resultColumns[0]->expr->column, "x");

    QVERIFY(core->resultColumns[1]->expr);
    QCOMPARE(core->resultColumns[1]->expr->column, "y");

    QVERIFY(core->resultColumns[2]->expr);
    QCOMPARE(core->resultColumns[2]->expr->function, "row_number");
    QVERIFY(core->resultColumns[2]->expr->filterOver);
    QCOMPARE(core->resultColumns[2]->expr->filterOver->over->name, "win1");

    QVERIFY(core->resultColumns[3]->expr);
    QCOMPARE(core->resultColumns[3]->expr->function, "rank");
    QVERIFY(core->resultColumns[3]->expr->filterOver);
    QCOMPARE(core->resultColumns[3]->expr->filterOver->over->name, "win2");

    // Windows
    QCOMPARE(core->windows.size(), 2);
    SqliteWindowDefinition* winDef1 = core->windows[0];
    QVERIFY(!winDef1->name.isNull());
    QVERIFY(winDef1->window);

    SqliteWindowDefinition::Window* win1 = winDef1->window;
    QVERIFY(win1->mode == SqliteWindowDefinition::Window::Mode::ORDER_BY);
    QVERIFY(win1->name.isNull());
    QVERIFY(win1->frame);
    QVERIFY(win1->orderBy.size() == 1);
    QVERIFY(win1->frame->rangeOrRows == SqliteWindowDefinition::Window::Frame::RangeOrRows::RANGE);
    QVERIFY(win1->frame->startBound);
    QVERIFY(win1->frame->startBound->type == SqliteWindowDefinition::Window::Frame::Bound::Type::UNBOUNDED_PRECEDING);
    QVERIFY(win1->frame->endBound);
    QVERIFY(win1->frame->endBound->type == SqliteWindowDefinition::Window::Frame::Bound::Type::CURRENT_ROW);

    SqliteWindowDefinition* winDef2 = core->windows[1];
    QVERIFY(!winDef2->name.isNull());
    QVERIFY(winDef2->window);

    SqliteWindowDefinition::Window* win2 = winDef2->window;
    QVERIFY(win2->mode == SqliteWindowDefinition::Window::Mode::PARTITION_BY);
    QVERIFY(win2->name.isNull());
    QVERIFY(win2->exprList.size() == 1);
    QVERIFY(win2->orderBy.size() == 1);
    QVERIFY(!win2->frame);

    ok = true;
}

void ParserTest::testFilterClause()
{
    QString sql = "SELECT c, a, b, group_concat(b, '.') FILTER (WHERE c!='two') OVER ("
                  "           ORDER BY a"
                  "       ) AS group_concat"
                  "  FROM t1 ORDER BY a;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteSelectPtr select = parser3->getQueries().first().dynamicCast<SqliteSelect>();
    QVERIFY(select->coreSelects.size() == 1);
    SqliteSelect::Core* core = select->coreSelects[0];
    QVERIFY(core->windows.size() == 0);

    QVERIFY(core->resultColumns.size() == 4);
    SqliteSelect::Core::ResultColumn* resCol = core->resultColumns[3];
    QVERIFY(resCol->alias == "group_concat");
    QVERIFY(resCol->expr);
    QVERIFY(resCol->expr->filterOver);
    QVERIFY(resCol->expr->filterOver->filter->expr);
    QVERIFY(resCol->expr->filterOver->over);
    QVERIFY(resCol->expr->filterOver->over->mode == SqliteFilterOver::Over::Mode::WINDOW);
    QVERIFY(resCol->expr->filterOver->over->window);
    QVERIFY(resCol->expr->filterOver->over->window->mode == SqliteWindowDefinition::Window::Mode::ORDER_BY);
}

void ParserTest::testFilterAsId()
{
    QString sql = "CREATE TABLE aa2 (sdfsdfdf, filter)";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteCreateTablePtr create = parser3->getQueries().first().dynamicCast<SqliteCreateTable>();
    QCOMPARE(create->columns.size(), 2);
    QCOMPARE(create->columns[1]->name, "filter");
}

void ParserTest::testUpdateFrom()
{
    QString sql = "UPDATE inventory"
                  "   SET quantity = quantity - daily.amt"
                  "  FROM (SELECT sum(quantity) AS amt, itemId FROM sales GROUP BY 2) AS daily"
                  " WHERE inventory.itemId = daily.itemId;";

    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteUpdatePtr update = parser3->getQueries().first().dynamicCast<SqliteUpdate>();
    QVERIFY(update->where);
    QCOMPARE(update->keyValueMap.size(), 1);
    QVERIFY(update->from);
    QVERIFY(update->from->singleSource);
    QVERIFY(update->from->singleSource->select);
    QCOMPARE(update->from->singleSource->alias, "daily");
}

void ParserTest::testStringAsTableId()
{
    QString sql = "select 'bb'.id1 = 'bb'.id2;";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());
}

void ParserTest::testJsonPtrOp()
{
    QString sql = "SELECT '[\"a11\", \"a22\", {\"x\":\"a33\"}]' -> 2,"
                  "       '[\"a11\", \"a22\", {\"x\":\"a33\"}]' ->> 2";
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());
}

void ParserTest::testUnfinishedSelectWithAliasForCompleter()
{
    QString sql = "select * from a1 x where x.";
    bool res = parser3->parse(sql, true);
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());
}

void ParserTest::testUnfinishedSelectWithAliasStrict()
{
    QString sql = "select * from a1 x where x.";
    bool res = parser3->parse(sql);
    QVERIFY(!res);
    QVERIFY(!parser3->getErrors().isEmpty());
}

void ParserTest::testBlobLiteral()
{
    QString sql = "insert into tab1 values (X'010e0F', 'string''with''quotes')";
    bool res = parser3->parse(sql);
    SqliteQueryPtr query = parser3->getQueries()[0];
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());

    SqliteInsertPtr insert = parser3->getQueries().first().dynamicCast<SqliteInsert>();
    SqliteSelect::Core* core = insert->select->coreSelects[0];
    QCOMPARE(core->resultColumns.size(), 2);

    QCOMPARE(core->resultColumns[0]->expr->mode, SqliteExpr::Mode::LITERAL_VALUE);
    QCOMPARE(core->resultColumns[0]->expr->literalValue.metaType(), QMetaType::fromType<QByteArray>());
    QCOMPARE(core->resultColumns[0]->expr->literalValue.toByteArray().toHex(), "010e0f");
    QCOMPARE(core->resultColumns[1]->expr->mode, SqliteExpr::Mode::LITERAL_VALUE);
    QCOMPARE(core->resultColumns[1]->expr->literalValue.metaType(), QMetaType::fromType<QString>());
    QCOMPARE(core->resultColumns[1]->expr->literalValue.toString(), "string''with''quotes");

    core->resultColumns[0]->expr->rebuildTokens();
    QCOMPARE(core->resultColumns[0]->expr->tokens[0]->value, "X'010e0f'");
}

void ParserTest::testBigDec()
{
    QString sql = "select 9999999999999999999 + 9999999999999999999, 9999999999999999999.1 + 9999999999999999999.2, 9999.1 + 9999.2;";
    bool res = parser3->parse(sql);
    SqliteQueryPtr query = parser3->getQueries()[0];
    QVERIFY(res);
    QVERIFY(parser3->getErrors().isEmpty());
    SqliteSelectPtr select = parser3->getQueries().first().dynamicCast<SqliteSelect>();
    SqliteSelect::Core* core = select->coreSelects[0];
    QCOMPARE(core->resultColumns[0]->expr->expr1->literalValue.metaType(), QMetaType::fromType<double>());
    QCOMPARE(core->resultColumns[1]->expr->expr1->literalValue.metaType(), QMetaType::fromType<double>());
    QCOMPARE(core->resultColumns[2]->expr->expr1->literalValue.metaType(), QMetaType::fromType<double>());
}

void ParserTest::testQuotedFunction()
{
    QString sql = "select \"abs\"(1)";
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

    QVERIFY(e->mode == SqliteExpr::Mode::FUNCTION);
    QCOMPARE(e->function, "abs");
}

void ParserTest::testIndexedSelect()
{
    QString sql = "select * from a1 indexed by i1";
    // parser3->setLemonDebug(true);
    bool res = parser3->parse(sql);
    QVERIFY(res);
    QVERIFY(parser3->getQueries().size() > 0);
}


void ParserTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    initUtilsSql();
    parser3 = new Parser();
}

void ParserTest::cleanupTestCase()
{
    delete parser3;
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
