#include "parser/keywords.h"
#include "parser/lexer.h"
#include "tablemodifier.h"
#include "parser/parser.h"
#include "db/db.h"
#include "dbsqlite3mock.h"
#include "mocks.h"
#include <QString>
#include <QtTest>
#include <QDebug>

class TableModifierTest : public QObject
{
        Q_OBJECT

    public:
        TableModifierTest();

    private:
        void verifyRe(const QString& re, const QString& sql, QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption);

        Db* db = nullptr;
        static const constexpr char* mainTableDdl = "CREATE TABLE test (id int, val text, val2 text);";
        SqliteCreateTablePtr createTable;

    private Q_SLOTS:
        void initTestCase();
        void init();
        void cleanup();
        void testCase1();
        void testCase2();
        void testCase3();
        void testCase4();
        void testCase5();
        void testCase6();
        void testCase7();
};

TableModifierTest::TableModifierTest()
{
}

void TableModifierTest::verifyRe(const QString& re, const QString& sql, QRegularExpression::PatternOptions options)
{
    QString fullMatchRe = QString("^%1$").arg(re);
    QRegularExpression regExp(fullMatchRe, options);
    QVERIFY2(regExp.match(sql).hasMatch(), QString("Failed RegExp validation:\n%1\nfor SQL:\n%2\n").arg(fullMatchRe).arg(sql).toLatin1().data());
}

void TableModifierTest::testCase1()
{
    db->exec("CREATE TABLE abc (id int, xyz text REFERENCES test (val));");

    TableModifier mod(db, "test");
    createTable->table = "test2";
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 1. Create new (with new name)
     * 2. Copy data to new
     * 4. Rename referencing table to temp name in 2 steps.
     * 5. Second step of renaming - drop old table.
     * 6. Create new referencing table.
     * 7. Copy data to new referencing table.
     * 8. Drop temp table.
     * 9. Drop old table.
     * 10. Re-enable FK.
     */
    QVERIFY(sqls.size() == 10);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE test2 .*", sqls[i++]);
    verifyRe("INSERT INTO test2.*SELECT.*FROM test;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*AS SELECT.*FROM abc.*", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("CREATE TABLE abc .*", sqls[i++]);
    verifyRe("INSERT INTO abc.*SELECT.*FROM sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("DROP TABLE sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase2()
{
    db->exec("CREATE TABLE abc (id int, xyz text REFERENCES test (val));");

    TableModifier mod(db, "test");
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Rename to temp in 2 steps.
     * 3. Second step of renaming (drop).
     * 4. Create new.
     * 5. Copy data from temp to new one.
     * 6. Rename referencing table to temp name in 2 steps.
     * 7. Second step of renaming (drop).
     * 8. Create new referencing table.
     * 9. Copy data to new referencing table.
     * 10. Drop first temp table.
     * 11. Drop second temp table.
     * 12. Enable FK.
     */
    QVERIFY(sqls.size() == 12);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*AS SELECT.*FROM test.*", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("CREATE TABLE test .*newCol.*", sqls[i++]);
    verifyRe("INSERT INTO test.*SELECT.*FROM sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*AS SELECT.*FROM abc.*", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("CREATE TABLE abc .*xyz text REFERENCES test \\(newCol\\).*", sqls[i++]);
    verifyRe("INSERT INTO abc.*SELECT.*FROM sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("DROP TABLE sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("DROP TABLE sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase3()
{
    db->exec("CREATE TABLE abc (id int, xyz text REFERENCES test (val));");
    db->exec("CREATE INDEX i1 ON test (val);");
    db->exec("CREATE INDEX i2 ON abc (id);");

    TableModifier mod(db, "test");
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Create new (with new name)
     * 3. Copy data to new
     * 4. Drop old table.
     * 5. Rename referencing table to temp name in two steps.
     * 6. Second step of renaming (drop).
     * 7. Create new referencing table.
     * 8. Copy data to new referencing table.
     * 9. Drop temp table.
     * 10. Re-create index i2.
     * 11. Re-create index i1 with new table name and column name.
     * 12. Disable FK.
     */
    QVERIFY(sqls.size() == 12);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable .*", sqls[i++]);
    verifyRe("INSERT INTO newTable.*SELECT.*FROM test;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*AS SELECT.*FROM abc.*", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("CREATE TABLE abc .*xyz text REFERENCES newTable \\(newCol\\).*", sqls[i++]);
    verifyRe("INSERT INTO abc.*SELECT.*FROM sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("DROP TABLE sqlitestudio_temp_table.*", sqls[i++]);
    verifyRe("CREATE INDEX i2 ON abc \\(id\\);", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("CREATE INDEX i1 ON newTable \\(newCol\\);", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase4()
{
    db->exec("CREATE TRIGGER t1 AFTER UPDATE OF Val ON Test BEGIN "
             "SELECT * FROM (SELECT Val FROM Test); "
             "UPDATE Test SET Val = (SELECT Val FROM Test) WHERE x = (SELECT Val FROM Test); "
             "INSERT INTO Test (val) VALUES (1); "
             "END;");

    TableModifier mod(db, "test");
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Create new (with new name)
     * 3. Copy data to new
     * 4. Drop old table.
     * 5. Recreate trigger with all subqueries updated.
     * 6. Enable FK.
     */
    QVERIFY(sqls.size() == 6);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable .*", sqls[i++]);
    verifyRe("INSERT INTO newTable.*SELECT.*FROM test;", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    QVERIFY2("CREATE TRIGGER t1 AFTER UPDATE OF newCol ON newTable "
             "BEGIN "
             "SELECT * FROM (SELECT newCol FROM newTable); "
             "UPDATE newTable SET newCol = (SELECT newCol FROM newTable) WHERE x = (SELECT newCol FROM newTable); "
             "INSERT INTO newTable (newCol) VALUES (1); "
             "END;" == sqls[i++], "Trigger DDL incorrect.");
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase5()
{
    db->exec("CREATE VIEW v1 AS SELECT * FROM (SELECT Val FROM Test);");
    db->exec("CREATE TRIGGER t1 INSTEAD OF INSERT ON v1 BEGIN SELECT 1; END;");
    db->exec("CREATE TRIGGER t2 AFTER INSERT ON v1 BEGIN SELECT 1; END;");
    db->exec("CREATE TRIGGER t3 INSTEAD OF INSERT ON test BEGIN SELECT 1; END;");

    TableModifier mod(db, "test");
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Create new (with new name)
     * 3. Copy data to new
     * 4. Drop old table.
     * 5. Drop old view.
     * 6. Recreate view with new column and table.
     * 7. Recreate trigger with all subqueries updated.
     * 8. Enable FK.
     */
    QVERIFY(sqls.size() == 8);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable .*", sqls[i++]);
    verifyRe("INSERT INTO newTable.*SELECT.*FROM test;", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("DROP VIEW v1;", sqls[i++]);
    verifyRe("CREATE VIEW v1 AS SELECT \\* FROM \\(SELECT newCol FROM newTable\\);", sqls[i++]);
    verifyRe("CREATE TRIGGER t1 INSTEAD OF INSERT ON v1 BEGIN SELECT 1; END;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase6()
{
    db->exec("CREATE VIEW v1 AS SELECT * FROM (SELECT Id, Val FROM Test);");
    db->exec("CREATE TRIGGER t2 AFTER UPDATE OF Id, Val ON Test BEGIN SELECT Val, Val2 FROM Test; END;");

    TableModifier mod(db, "test");
    createTable->table = "newTable";
    createTable->columns.removeAt(1);
    mod.alterTable(createTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Create new (with new name)
     * 3. Copy data to new
     * 4. Drop old table.
     * 5. Recreate trigger with all subqueries updated.
     * 6. Drop view.
     * 7. Recreate view with new table referenced.
     * 8. Enable FK.
     */
    QVERIFY(sqls.size() == 8);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable \\(id int, val2 text\\);", sqls[i++]);
    verifyRe("INSERT INTO newTable \\(id, val2\\) SELECT id, val2 FROM test;", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("CREATE TRIGGER t2 AFTER UPDATE OF Id ON newTable BEGIN SELECT NULL, Val2 FROM newTable; END;", sqls[i++]);
    verifyRe("DROP VIEW v1;", sqls[i++]);
    verifyRe("CREATE VIEW v1 AS SELECT \\* FROM \\(SELECT Id, NULL FROM newTable\\);", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase7()
{
    static_qstring(ddl, "CREATE TABLE abc (col1 text, col2 as (col1 || 'x'));");
    db->exec(ddl);

    Parser parser;
    Q_ASSERT(parser.parse(ddl));
    Q_ASSERT(parser.getQueries().size() > 0);
    SqliteCreateTablePtr localCreateTable = parser.getQueries().first().dynamicCast<SqliteCreateTable>();
    Q_ASSERT(!createTable.isNull());


    TableModifier mod(db, "abc");
    localCreateTable->table = "newTable";
    mod.alterTable(localCreateTable);
    QStringList sqls = mod.generateSqls();

    /*
     * 1. Disable FK.
     * 2. Create new (with new name)
     * 3. Copy data to new
     * 4. Drop old table.
     * 5. Enable FK.
     */
    QVERIFY(sqls.size() == 5);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable \\(col1 text, col2 AS \\(col1 \\|\\| 'x'\\)\\);", sqls[i++]);
    verifyRe("INSERT INTO newTable \\(col1\\) SELECT col1 FROM abc\\;", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
}

void TableModifierTest::init()
{
    initMocks();
    initUtilsSql();

    db = new DbSqlite3Mock("testdb");
    db->open();
    db->exec(mainTableDdl);

    Parser parser;
    Q_ASSERT(parser.parse(mainTableDdl));
    Q_ASSERT(parser.getQueries().size() > 0);
    createTable = parser.getQueries().first().dynamicCast<SqliteCreateTable>();
    Q_ASSERT(!createTable.isNull());
}

void TableModifierTest::cleanup()
{
    db->close();
    delete db;
    db = nullptr;
}


QTEST_APPLESS_MAIN(TableModifierTest)

#include "tst_tablemodifiertest.moc"
