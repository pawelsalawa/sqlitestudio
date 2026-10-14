#include "importworker.h"
#include "plugins/genericplugin.h"
#include "plugins/importplugin.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "dbsqlite3mock.h"
#include "mocks.h"
#include <QtTest>

// Exercise the shared row contract, independent of source format.
class RowImportPlugin : public GenericPlugin, public ImportPlugin
{
    public:
        QString getDataSourceTypeName() const override {return "Test";}
        ImportManager::StandardConfigFlags standardOptionsToEnable() const override {return {};}
        QString getFileFilter() const override {return {};}
        bool beforeImport(const ImportManager::StandardImportConfig&) override {return true;}
        void afterImport() override {cleaned = true;}
        QList<ColumnDefinition> getColumns() const override {return {{"value", "INTEGER"}};}
        QList<QVariant> next() override {return rows.isEmpty() ? QList<QVariant>() : rows.takeFirst();}
        CfgMain* getConfig() override {return nullptr;}
        QString getImportConfigFormName() const override {return {};}
        bool validateOptions() override {return true;}

        QList<QList<QVariant>> rows;
        bool cleaned = false;
};

class ImportWorkerTest : public QObject
{
    Q_OBJECT

    private slots:
        void initTestCase()
        {
            initKeywords();
            Lexer::staticInit();
            initMocks();
            initUtilsSql();
        }

        void constraintError_data()
        {
            QTest::addColumn<bool>("ignoreErrors");
            QTest::addColumn<bool>("noDbLock");
            QTest::addColumn<int>("failedRow");
            QTest::newRow("dialog-first") << false << false << 1;
            QTest::newRow("dialog-later") << false << false << 3;
            QTest::newRow("ignore-errors") << true << false << 3;
            QTest::newRow("sql-import") << true << true << 3;
        }

        void constraintError()
        {
            QFETCH(bool, ignoreErrors);
            QFETCH(bool, noDbLock);
            QFETCH(int, failedRow);
            DbSqlite3Mock db("import-test");
            QVERIFY(db.open());
            QVERIFY(!db.exec("CREATE TABLE target (value INTEGER CHECK(value > 0))")->isError());

            RowImportPlugin plugin;
            for (int i = 1; i < failedRow; i++)
                plugin.rows << QList<QVariant>{i};

            plugin.rows << QList<QVariant>{-1} << QList<QVariant>{-2} << QList<QVariant>{42};
            ImportManager::StandardImportConfig config;
            config.ignoreErrors = ignoreErrors;
            config.noDbLock = noDbLock;
            ImportWorker worker(&plugin, &config, &db, "target");
            QSignalSpy errors(NOTIFY_MANAGER, &NotifyManager::notifyError);
            QSignalSpy warnings(NOTIFY_MANAGER, &NotifyManager::notifyWarning);
            QSignalSpy finished(&worker, &ImportWorker::finished);
            worker.run();

            QCOMPARE(finished.size(), 1);
            QCOMPARE(finished.first().first().toBool(), ignoreErrors);
            QVERIFY(plugin.cleaned);
            QCOMPARE(errors.size(), ignoreErrors ? 0 : 1);
            QCOMPARE(warnings.size(), ignoreErrors ? 2 : 0);
            const QSignalSpy& messages = ignoreErrors ? warnings : errors;
            for (int i = 0; i < messages.size(); i++)
            {
                QString message = messages[i].first().toString();
                QVERIFY2(message.contains(QString("row number %1").arg(failedRow + i)), qPrintable(message));
                QVERIFY2(message.contains("CHECK constraint failed"), qPrintable(message));
            }
            SqlQueryPtr count = db.exec("SELECT count(*) FROM target");
            QVERIFY(count->hasNext());
            QCOMPARE(count->next()->value(0).toInt(), ignoreErrors ? failedRow : 0);
            count.clear();
            db.close();
        }
};

QTEST_APPLESS_MAIN(ImportWorkerTest)
#include "tst_importworkertest.moc"
