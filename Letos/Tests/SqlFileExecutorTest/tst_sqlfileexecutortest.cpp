#include "dbsqlite3mock.h"
#include "mocks.h"
#include "parser/keywords.h"
#include "sqlfileexecutor.h"
#include "common/collections.h"
#include "parser/lexer.h"
#include <QSignalSpy>
#include <QTest>

// add necessary includes here

class SqlFileExecutorTest : public QObject
{
        Q_OBJECT

    public:
        SqlFileExecutorTest();
        ~SqlFileExecutorTest();

    private:
        void printErrors(QSignalSpy& spyErrors);

        Db* db = nullptr;

    private slots:
        void initTestCase();
        void init();
        void cleanup();
        void cleanupTestCase();
        void testPrintExtended();
        void testPrintStrict();
        void testPrintPermissive();
        void testRead1();
        void testRead2();

};

SqlFileExecutorTest::SqlFileExecutorTest()
{

}

SqlFileExecutorTest::~SqlFileExecutorTest()
{

}

void SqlFileExecutorTest::printErrors(QSignalSpy& spyErrors)
{
    if (spyErrors.count() > 0)
    {
        QList<QVariant> args = spyErrors[0];
        QList<QPair<QString, QString>> errors = qvariant_cast<QList<QPair<QString, QString>>>(args[0]);
        for (auto& err : errors)
            qWarning() << "Error executing query:" << err.first << ", error:" << err.second;
    }
}

void SqlFileExecutorTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    initMocks();
}

void SqlFileExecutorTest::init()
{
    db = new DbSqlite3Mock("testdb");
    db->open();
}

void SqlFileExecutorTest::cleanup()
{
    db->close();
    delete db;
    db = nullptr;
}

void SqlFileExecutorTest::cleanupTestCase()
{
}

void SqlFileExecutorTest::testPrintExtended()
{
    SqlFileExecutor fileExecutor;
    fileExecutor.setExecutionMode(SqlFileExecutor::EXTENDED);
    QSignalSpy spyErrors(&fileExecutor, &SqlFileExecutor::execErrors);
    QSignalSpy spyNotifyInfo(NOTIFY_MANAGER, &NotifyManager::notifyInfo);

    fileExecutor.execSqlFromFile(db, ":/sql/testPrint.sql", false, defaultCodecName(), false);

    printErrors(spyErrors);
    QVERIFY(spyErrors.count() == 0);

    QStringList infoMsgs = spyNotifyInfo | MAP(nfo, {return nfo[0].toString();});
    QVERIFY(infoMsgs.contains("x1"));
    QVERIFY(!infoMsgs.contains("x2"));
    QVERIFY(infoMsgs.contains("x3"));
}

void SqlFileExecutorTest::testPrintStrict()
{
    SqlFileExecutor fileExecutor;
    fileExecutor.setExecutionMode(SqlFileExecutor::STRICT_MODE);
    QSignalSpy spyErrors(&fileExecutor, &SqlFileExecutor::execErrors);
    QSignalSpy spyNotifyInfo(NOTIFY_MANAGER, &NotifyManager::notifyInfo);

    fileExecutor.execSqlFromFile(db, ":/sql/testPrint.sql", false, defaultCodecName(), false);

    printErrors(spyErrors);
    QVERIFY(spyErrors.count() == 1);
}

void SqlFileExecutorTest::testPrintPermissive()
{
    SqlFileExecutor fileExecutor;
    fileExecutor.setExecutionMode(SqlFileExecutor::PERMISSIVE);
    QSignalSpy spyErrors(&fileExecutor, &SqlFileExecutor::execErrors);
    QSignalSpy spyNotifyInfo(NOTIFY_MANAGER, &NotifyManager::notifyInfo);

    fileExecutor.execSqlFromFile(db, ":/sql/testPrint.sql", false, defaultCodecName(), false);

    printErrors(spyErrors);
    QVERIFY(spyErrors.count() == 0);

    QStringList infoMsgs = spyNotifyInfo | MAP(nfo, {return nfo[0].toString();});
    QVERIFY(!infoMsgs.contains("x1"));
    QVERIFY(!infoMsgs.contains("x2"));
    QVERIFY(!infoMsgs.contains("x3"));
}

void SqlFileExecutorTest::testRead1()
{
    SqlFileExecutor fileExecutor;
    fileExecutor.setExecutionMode(SqlFileExecutor::EXTENDED);
    QSignalSpy spyErrors(&fileExecutor, &SqlFileExecutor::execErrors);
    QSignalSpy spyNotifyInfo(NOTIFY_MANAGER, &NotifyManager::notifyInfo);

    fileExecutor.execSqlFromFile(db, ":/sql/testRead1.sql", false, defaultCodecName(), false);

    printErrors(spyErrors);
    QVERIFY(spyErrors.count() == 0);

    QStringList infoMsgs = spyNotifyInfo | MAP(nfo, {return nfo[0].toString();});
    QVERIFY(infoMsgs.contains("expected value 1 2 3"));
}

void SqlFileExecutorTest::testRead2()
{
    SqlFileExecutor fileExecutor;
    fileExecutor.setExecutionMode(SqlFileExecutor::EXTENDED);
    QSignalSpy spyErrors(&fileExecutor, &SqlFileExecutor::execErrors);
    QSignalSpy spyNotifyInfo(NOTIFY_MANAGER, &NotifyManager::notifyInfo);

    fileExecutor.execSqlFromFile(db, ":/sql/testRead2.sql", false, defaultCodecName(), false);

    printErrors(spyErrors);
    QVERIFY(spyErrors.count() == 1);
}

QTEST_APPLESS_MAIN(SqlFileExecutorTest)

#include "tst_sqlfileexecutortest.moc"
