#include <QString>
#include <QtTest>
#include "common/utils_sql.h"

class UtilsSqlTest : public QObject
{
    Q_OBJECT

public:
    UtilsSqlTest();

private Q_SLOTS:
    void testCaseDefault();
    void testRemoveEmpties();
    void testRemoveComments();
    void testRemoveCommentsAndEmpties();
};

UtilsSqlTest::UtilsSqlTest()
{
}

void UtilsSqlTest::testCaseDefault()
{
    QString sql = "select 'dfgh ;sdg '' dfga' from aa; insert into x values ('sdg', ';drghd;;;''', 4); select 1, ''; select 2;";
    QStringList sp = quickSplitQueries(sql);

    QString failure = "Failure, got: \"%1\"";

    QVERIFY2(sp.size() == 4, failure.arg(sp.size()).toLatin1().data());
    QVERIFY2(sp[0] == "select 'dfgh ;sdg '' dfga' from aa;", failure.arg(sp[0]).toLatin1().data());
    QVERIFY2(sp[1] == " insert into x values ('sdg', ';drghd;;;''', 4);", failure.arg(sp[1]).toLatin1().data());
    QVERIFY2(sp[2] == " select 1, '';", failure.arg(sp[2]).toLatin1().data());
    QVERIFY2(sp[3] == " select 2;", failure.arg(sp[3]).toLatin1().data());
}

void UtilsSqlTest::testRemoveEmpties()
{
    QString sql = "select 'dfgh ;sdg '' dfga' from aa; ; select 1, '';";
    QStringList sp = quickSplitQueries(sql, false);

    QString failure = "Failure, got: \"%1\"";

    QVERIFY2(sp.size() == 2, failure.arg(sp.size()).toLatin1().data());
    QVERIFY2(sp[0] == "select 'dfgh ;sdg '' dfga' from aa;", failure.arg(sp[0]).toLatin1().data());
    QVERIFY2(sp[1] == " select 1, '';", failure.arg(sp[1]).toLatin1().data());
}

void UtilsSqlTest::testRemoveComments()
{
    QString sql = "select 'dfgh ;sdg '' dfga' from aa; select 1/*, ''*/;--select 1\nselect 2;";
    QStringList sp = quickSplitQueries(sql, true, true);

    QString failure = "Failure, got: \"%1\"";

    QVERIFY2(sp.size() == 3, failure.arg(sp.size()).toLatin1().data());
    QVERIFY2(sp[0] == "select 'dfgh ;sdg '' dfga' from aa;", failure.arg(sp[0]).toLatin1().data());
    QVERIFY2(sp[1] == " select 1;", failure.arg(sp[1]).toLatin1().data());
    QVERIFY2(sp[2] == "select 2;", failure.arg(sp[2]).toLatin1().data());
}

void UtilsSqlTest::testRemoveCommentsAndEmpties()
{
    QString sql = "select 'dfgh ;sdg /*''*/ dfga' from aa; /*select 1, ''*/;--select 1\n--select 2;";
    QStringList sp = quickSplitQueries(sql, false, true);

    QString failure = "Failure, got: \"%1\"";

    QVERIFY2(sp.size() == 1, failure.arg(sp.size()).toLatin1().data());
    QVERIFY2(sp[0] == "select 'dfgh ;sdg /*''*/ dfga' from aa;", failure.arg(sp[0]).toLatin1().data());
}

QTEST_APPLESS_MAIN(UtilsSqlTest)

#include "tst_utilssqltest.moc"
