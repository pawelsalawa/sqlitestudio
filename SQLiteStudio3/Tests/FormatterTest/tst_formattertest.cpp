#include "plugins/codeformatterplugin.h"
#include "parser/lexer.h"
#include "parser/keywords.h"
#include "common/utils_sql.h"
#include "common/global.h"
#include "db/dbsqlite3.h"
#include "mocks.h"
#include <QString>
#include <QtTest>

class FormatterTest : public QObject
{
        Q_OBJECT

    public:
        FormatterTest();

    private:
        QPluginLoader* loader = nullptr;
        CodeFormatterPlugin* plugin = nullptr;
        Db* db = nullptr;

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();

        void test1();
        void test2();
        void test3();
};

FormatterTest::FormatterTest()
{
}

void FormatterTest::test1()
{
    QString sql = "SELECT 1";
    QString formatted = plugin->format(sql, db);
    QCOMPARE(formatted, "SELECT 1;\n");
}

void FormatterTest::test2()
{
    QString sql = "SELECT ';';";
    QString formatted = plugin->format(sql, db);
    QCOMPARE(formatted, "SELECT ';';\n");
}

void FormatterTest::test3()
{
    QString sql = "SELECT * from test;";
    QString formatted = plugin->format(sql, db);
    QCOMPARE(formatted, "SELECT *\n  FROM test;\n");
}

void FormatterTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    initUtilsSql();
    initMocks();

    QStringList nameFilters = {"*SqlEnterpriseFormatter*.so", "*SqlEnterpriseFormatter*.dll", "*SqlEnterpriseFormatter*.dylib"};

    QDir pluginDir("plugins");
    QStringList files = pluginDir.entryList(nameFilters, QDir::Files);
    QCOMPARE(files.size(), 1);

    QString fileName = pluginDir.absoluteFilePath(files.first());
    loader = new QPluginLoader(fileName);
    loader->setLoadHints(QLibrary::ExportExternalSymbolsHint|QLibrary::ResolveAllSymbolsHint);
    QVERIFY(loader->load());

    plugin = dynamic_cast<CodeFormatterPlugin*>(loader->instance());
    QVERIFY(plugin);

    db = new DbSqlite3("test", ":memory:", {{DB_PURE_INIT, true}});
}

void FormatterTest::cleanupTestCase()
{
    if (loader)
    {
        if (loader->isLoaded())
            loader->unload();

        safe_delete(loader);
    }
}

QTEST_APPLESS_MAIN(FormatterTest)

#include "tst_formattertest.moc"
