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
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test renamed to test2.
     *
     * 1. Enable legacy alter table.
     * 2. Disable FK.
     * 3. Create new (with new name)
     * 4. Copy data to new
     * 5. Drop old table.
     * 6. Create new structure of referencing table using temp name.
     * 7. Copy referencing table data into new temp table.
     * 8. Drop old referencing table.
     * 9. Rename temp referencing table to its original name.
     * 10. Re-enable FK.
     * 11. Disable legacy alter table.
     */
    QVERIFY(sqls.size() == 11);
    int i = 0;
    verifyRe("PRAGMA legacy_alter_table = true;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE test2 .*", sqls[i++]);
    verifyRe("INSERT INTO test2.*SELECT.*FROM test;", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.* REFERENCES test2.*", sqls[i++]);
    verifyRe("INSERT INTO sqlitestudio_temp_table.*SELECT .* FROM abc.*", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("ALTER TABLE sqlitestudio_temp_table RENAME TO abc;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
    verifyRe("PRAGMA legacy_alter_table = false;", sqls[i++]);
}

void TableModifierTest::testCase2()
{
    db->exec("CREATE TABLE abc (id int, xyz text REFERENCES test (val));");

    TableModifier mod(db, "test");
    createTable->columns[1]->name = "newCol";
    mod.setUseLegacyAlterRename(false);
    mod.alterTable(createTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test column 'val' renamed to 'newCol'.
     *
     * 1. Disable FK.
     * 2. Create new structure under temp table name (with new column name).
     * 3. Copy data to new temp table.
     * 4. Drop old table.
     * 5. Rename temp table to original name.
     * 6. Create new structure of referencing table using temp name (with new column name in REFERENCES).
     * 7. Copy referencing table data into new temp table.
     * 8. Drop old referencing table.
     * 9. Rename temp referencing table to its original name.
     * 10. Enable FK.
     */
    QVERIFY(sqls.size() == 10);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*newCol.*", sqls[i++]);
    verifyRe("INSERT INTO sqlitestudio_temp_table.*SELECT.*FROM test.*", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("ALTER TABLE sqlitestudio_temp_table RENAME TO test;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table0.*REFERENCES test.*newCol.*", sqls[i++]);
    verifyRe("INSERT INTO sqlitestudio_temp_table0.*SELECT.*FROM abc.*", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("ALTER TABLE sqlitestudio_temp_table0 RENAME TO abc;", sqls[i++]);
    verifyRe("PRAGMA foreign_keys = 1;", sqls[i++]);
}

void TableModifierTest::testCase3()
{
    db->exec("CREATE TABLE abc (id int, xyz text REFERENCES test (val));");
    db->exec("CREATE INDEX i1 ON test (val);");
    db->exec("CREATE INDEX i2 ON abc (id);");

    TableModifier mod(db, "test");
    mod.setUseLegacyAlterRename(false);
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test renamed to newTable and column 'val' renamed to 'newCol'.
     *
     * 1. Disable FK.
     * 2. Create new (with new name and new column name).
     * 3. Copy data to new.
     * 4. Drop old table.
     * 5. Create new structure of referencing table using temp name (with new column name in REFERENCES).
     * 6. Copy referencing table data into new temp table.
     * 7. Drop old referencing table.
     * 8. Rename temp referencing table to its original name.
     * 9. Recreate index on referencing table.
     * 10. Recreate index on new table.
     * 11. Enable FK.
     */
    QVERIFY(sqls.size() == 11);
    int i = 0;
    verifyRe("PRAGMA foreign_keys = 0;", sqls[i++]);
    verifyRe("CREATE TABLE newTable .*newCol.*", sqls[i++]);
    verifyRe("INSERT INTO newTable.*newCol.*SELECT.*val,.*FROM test;", sqls[i++]);
    verifyRe("DROP TABLE test;", sqls[i++]);
    verifyRe("CREATE TABLE sqlitestudio_temp_table.*REFERENCES newTable.*newCol.*", sqls[i++]);
    verifyRe("INSERT INTO sqlitestudio_temp_table.*SELECT.*FROM abc;", sqls[i++]);
    verifyRe("DROP TABLE abc;", sqls[i++]);
    verifyRe("ALTER TABLE sqlitestudio_temp_table RENAME TO abc;", sqls[i++]);
    verifyRe("CREATE INDEX i2 ON abc \\(id\\);", sqls[i++]);
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
    mod.setUseLegacyAlterRename(false);
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test renamed to newTable and column 'val' renamed to 'newCol'.
     *
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
    mod.setUseLegacyAlterRename(false);
    createTable->table = "newTable";
    createTable->columns[1]->name = "newCol";
    mod.alterTable(createTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test renamed to newTable and column 'val' renamed to 'newCol'.
     *
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
    mod.setUseLegacyAlterRename(false);
    createTable->table = "newTable";
    createTable->columns.removeAt(1);
    mod.alterTable(createTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table test renamed to newTable and column 'val' removed.
     *
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
    mod.setUseLegacyAlterRename(false);
    localCreateTable->table = "newTable";
    mod.alterTable(localCreateTable);
    QStringList sqls = mod.getGeneratedSqls();

    /*
     * Table abc renamed to newTable (with generated column).
     *
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
