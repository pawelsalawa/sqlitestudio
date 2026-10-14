#include "letos.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "config_builder/cfgmain.h"
#include "config_builder/cfglazyinitializer.h"
#include "db/dbsqlite3.h"
#include "db/queryexecutor.h"
#include "db/queryexecutorsteps/queryexecutorstep.h"
#include "impl/dbattacherimpl.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "plugins/dbpluginsqlite3.h"
#include "schemaresolver.h"
#include "services/impl/collationmanagerimpl.h"
#include "services/impl/dbmanagerimpl.h"
#include "services/impl/functionmanagerimpl.h"
#include "services/impl/pluginmanagerimpl.h"
#include "configmock.h"
#include "extensionmanagermock.h"
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using Rows = QList<QVariantList>;

static QtMessageHandler previousMessageHandler = nullptr;

class FailMetadataStep : public QueryExecutorStep
{
    public:
        bool exec() override {return false;}
};

class QueryExecutorTest : public QObject
{
    Q_OBJECT

    private:
        bool execSql(Db* db, const QString& sql);
        Rows readRows(SqlQueryPtr results, const QStringList& columns = {});
        void addQueryRows(const QList<QPair<QString, QString>>& queries,
                          const QStringList& objects = {"btable", "RegInfo", "AllNames", "CteInfo"});
        void checkImplicitSelect(bool expectFallback = false);
        void checkSelect(const QString& sql, bool noMetaColumns = false, bool expectFallback = false);

        QScopedPointer<QTemporaryDir> directory;
        Db* db = nullptr;

    private slots:
        void initTestCase();
        void init();
        void cleanup();
        void cleanupTestCase();
        void implicitSelect_data();
        void implicitSelect();
        void implicitCteDml_data();
        void implicitCteDml();
        void cteControls_data();
        void cteControls();
        void viewScopes_data();
        void viewScopes();
        void metadataFallback_data();
        void metadataFallback();
        void nestedRowIds_data();
        void nestedRowIds();
};

bool QueryExecutorTest::execSql(Db* database, const QString& sql)
{
    SqlQueryPtr results = database->exec(sql, Db::Flag::PRELOAD);
    if (results->isError())
        qWarning() << sql << results->getErrorText();

    return !results->isError();
}

Rows QueryExecutorTest::readRows(SqlQueryPtr results, const QStringList& columns)
{
    Rows rows;
    for (const SqlResultsRowPtr& row : results->getAll())
    {
        QVariantList values;
        if (columns.isEmpty())
            values = row->valueList();
        else
            for (const QString& column : columns)
                values << row->value(column);

        rows << values;
    }
    return rows;
}

void QueryExecutorTest::initTestCase()
{
    previousMessageHandler = qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        if (type == QtCriticalMsg)
            QTest::qFail(qPrintable(message), context.file ? context.file : __FILE__, context.line);

        if (previousMessageHandler)
            previousMessageHandler(type, context, message);
    });
    // Keep settings/extensions isolated, but exercise real registry, driver and attacher.
    LETOS->setConfig(new ConfigMock());
    CfgLazyInitializer::init();
    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    SchemaResolver::staticInit();
    initKeywords();
    Lexer::staticInit();
    LETOS->setFunctionManager(new FunctionManagerImpl());
    LETOS->setCollationManager(new CollationManagerImpl());
    LETOS->setSqliteExtensionManager(new ExtensionManagerMock());
    LETOS->setDbAttacherFactory(new DbAttacherDefaultFactory());
    auto plugins = new PluginManagerImpl();
    LETOS->setPluginManager(plugins);
    auto manager = new DbManagerImpl();
    LETOS->setDbManager(manager);
    manager->init();
    static_cast<PluginManager*>(plugins)->registerPluginType<DbPlugin>("Database support");
    QVERIFY(plugins->loadBuiltInPlugin(new DbPluginSqlite3()));
}

void QueryExecutorTest::init()
{
    bool initialized = false;
    auto cleanupOnFailure = qScopeGuard([&] {if (!initialized) cleanup();});
    directory.reset(new QTemporaryDir());
    QVERIFY(directory->isValid());
    for (int index = 1; index <= 3; index++)
    {
        QString name = QString("db%1").arg(index);
        QString path = directory->filePath(name + ".db");
        DbSqlite3 fixture(name, path);
        QVERIFY(fixture.open());
        if (index == 1)
        {
            QVERIFY(execSql(&fixture, "CREATE TABLE atable(key INTEGER PRIMARY KEY, Amount INTEGER)"));
            QVERIFY(execSql(&fixture, "INSERT INTO atable VALUES(1,100),(2,200),(3,300),(4,400)"));
            QVERIFY(execSql(&fixture, "CREATE TABLE destination(key INTEGER PRIMARY KEY, Amount INTEGER)"));
        }
        else
        {
            QVERIFY(execSql(&fixture, "CREATE TABLE btable(key INTEGER PRIMARY KEY, parent INTEGER, Amount INTEGER)"));
            QVERIFY(execSql(&fixture, index == 2 ?
                               "INSERT INTO btable VALUES(1,NULL,10),(2,1,20),(3,2,30)" :
                               "INSERT INTO btable VALUES(4,NULL,40)"));
            QVERIFY(execSql(&fixture, "CREATE VIEW RegInfo AS SELECT * FROM btable"));
            QVERIFY(execSql(&fixture, "CREATE VIEW AllNames AS SELECT * FROM RegInfo"));
            QVERIFY(execSql(&fixture, "CREATE VIEW CteInfo AS WITH src AS (SELECT * FROM btable) SELECT * FROM src"));
            QVERIFY(execSql(&fixture, "CREATE TABLE keys(key INTEGER)"));
            QVERIFY(execSql(&fixture, "INSERT INTO keys VALUES(1),(3)"));
            QVERIFY(execSql(&fixture, "CREATE VIEW JoinedView AS SELECT b.key,b.parent,b.Amount FROM btable b"
                                     " JOIN keys k ON k.key=b.key"));
            QVERIFY(execSql(&fixture, "CREATE VIEW ScopedView AS WITH keys AS (SELECT key FROM btable)"
                                     " SELECT * FROM btable WHERE key IN keys"));
            QVERIFY(execSql(&fixture, "CREATE VIEW InView AS SELECT * FROM btable WHERE key IN keys"));
            QVERIFY(execSql(&fixture, "CREATE VIEW NestedView AS SELECT * FROM (SELECT * FROM btable) src"
                                     " WHERE EXISTS (SELECT 1 FROM btable inner_src WHERE inner_src.key=src.key)"));
        }
        QVERIFY(fixture.close());
        QVERIFY(DBLIST->addDb(name, path, false));
    }
    db = DBLIST->getByName("db1");
    QVERIFY(db);
    QVERIFY(db->open());
    initialized = true;
}

void QueryExecutorTest::cleanup()
{
    for (Db* database : DBLIST->getDbList())
        DBLIST->removeDb(database);

    db = nullptr;
    directory.reset();
}

void QueryExecutorTest::cleanupTestCase()
{
    LETOS->setDbManager(nullptr);
    LETOS->getPluginManager()->deinit();
    LETOS->setPluginManager(nullptr);
    LETOS->setDbAttacherFactory(nullptr);
    LETOS->setFunctionManager(nullptr);
    LETOS->setCollationManager(nullptr);
    LETOS->setSqliteExtensionManager(nullptr);
    LETOS->setConfig(nullptr);
    qInstallMessageHandler(previousMessageHandler);
}

void QueryExecutorTest::addQueryRows(const QList<QPair<QString, QString>>& queries, const QStringList& objects)
{
    QTest::addColumn<QString>("sql");
    QTest::addColumn<bool>("connected");
    QTest::addColumn<bool>("sameNamedTable");
    for (const auto& query : queries)
        for (const QString& object : objects)
            for (bool connected : {false, true})
                for (bool sameNamedTable : {false, true})
                {
                    QString tag = QString("%1/%2/%3/%4").arg(query.first, object,
                                      connected ? "connected" : "disconnected",
                                      sameNamedTable ? "main-collision" : "no-collision");
                    QTest::newRow(qPrintable(tag)) << query.second.arg("db2." + object)
                                                   << connected << sameNamedTable;
                }
}

void QueryExecutorTest::implicitSelect_data()
{
    // Ordinary, nested and recursive WITH forms from https://sqlite.org/lang_with.html.
    // Every statement is also executed directly by SQLite to validate syntax and values.
    addQueryRows({
        {"direct-star", "SELECT * FROM %1 ORDER BY key"},
        {"plain-star", "SELECT *, 'abc' AS stuff FROM %1 ORDER BY key"},
        {"alias-star", "SELECT ri.*, 'abc' AS stuff FROM %1 ri ORDER BY ri.key"},
        {"explicit-column", "SELECT ri.Amount, 'abc' AS stuff FROM %1 ri ORDER BY ri.key"},
        {"database-name-alias", "SELECT db2.key,db2.Amount FROM %1 db2 ORDER BY db2.key"},
        {"not-exists", "SELECT a.key, a.Amount FROM atable a WHERE NOT EXISTS"
                       " (SELECT 1 FROM %1 t2 WHERE t2.key=a.key) ORDER BY a.key"},
        {"join", "SELECT a.key, ri.Amount FROM atable a JOIN %1 ri ON ri.key=a.key ORDER BY a.key"},
        {"scalar-subquery", "SELECT a.key, (SELECT ri.Amount FROM %1 ri WHERE ri.key=a.key) AS amount"
                            " FROM atable a ORDER BY a.key"},
        {"cte-single", "WITH src AS (SELECT key, Amount FROM %1) SELECT * FROM src ORDER BY key"},
        {"cte-view-shadow", "WITH btable AS (SELECT key,Amount FROM atable)"
                            " SELECT ri.key,ri.Amount FROM %1 ri ORDER BY ri.key"},
        {"cte-alias", "WITH src AS (SELECT key,Amount FROM %1) SELECT s.key,s.Amount FROM src s ORDER BY s.key"},
        {"cte-column-list", "WITH src(k,v) AS (SELECT key, Amount FROM %1) SELECT k,v FROM src ORDER BY k"},
        {"cte-chain", "WITH src AS (SELECT * FROM %1), middle AS (SELECT key,Amount FROM src),"
                      " final AS (SELECT * FROM middle WHERE Amount>=20) SELECT * FROM final ORDER BY key"},
        {"cte-forward-reference", "WITH final AS (SELECT key,Amount FROM src), src AS (SELECT * FROM %1)"
                                  " SELECT * FROM final ORDER BY key"},
        {"cte-independent", "WITH left_side AS (SELECT key FROM %1), right_side AS (SELECT key,Amount FROM %1)"
                            " SELECT l.key,r.Amount FROM left_side l JOIN right_side r ON l.key=r.key ORDER BY l.key"},
        {"cte-independent-no-alias", "WITH left_side AS (SELECT key FROM %1), right_side AS (SELECT key,Amount FROM %1)"
                                     " SELECT left_side.key,right_side.Amount FROM left_side JOIN right_side"
                                     " ON left_side.key=right_side.key ORDER BY left_side.key"},
        {"cte-two-databases", "WITH src AS (SELECT key,Amount FROM %1), other AS (SELECT key,Amount FROM db3.btable)"
                              " SELECT * FROM src UNION ALL SELECT * FROM other ORDER BY key"},
        {"cte-nested-body", "WITH outer_src AS (WITH inner_src AS (SELECT key,Amount FROM %1)"
                            " SELECT * FROM inner_src) SELECT * FROM outer_src ORDER BY key"},
        {"cte-nested-from", "SELECT * FROM (WITH src AS (SELECT key,Amount FROM %1) SELECT * FROM src) ORDER BY key"},
        {"cte-nested-scalar", "SELECT a.key, (WITH src AS (SELECT key,Amount FROM %1)"
                              " SELECT Amount FROM src WHERE src.key=a.key) AS amount FROM atable a ORDER BY a.key"},
        {"cte-nested-exists", "SELECT a.key FROM atable a WHERE NOT EXISTS (WITH src AS (SELECT key FROM %1)"
                              " SELECT 1 FROM src WHERE src.key=a.key) ORDER BY a.key"},
        {"cte-shadowed-scope", "WITH src AS (SELECT key,Amount FROM %1), final AS"
                               " (WITH src AS (SELECT key,Amount FROM atable) SELECT * FROM src)"
                               " SELECT * FROM final UNION ALL SELECT * FROM src ORDER BY key,Amount"},
        {"cte-table-name", "WITH btable AS (SELECT key,Amount FROM %1) SELECT btable.* FROM btable ORDER BY key"},
        {"cte-database-name", "WITH db2 AS (SELECT key,Amount FROM %1) SELECT db2.key,db2.Amount FROM db2 ORDER BY key"},
        {"cte-quoted-database-name", "WITH [DB2] AS (SELECT key,Amount FROM %1)"
                                     " SELECT [db2].key,[db2].Amount FROM [Db2] ORDER BY key"},
        {"cte-materialized", "WITH src AS MATERIALIZED (SELECT key,Amount FROM %1) SELECT * FROM src ORDER BY key"},
        {"cte-not-materialized", "WITH src AS NOT MATERIALIZED (SELECT key,Amount FROM %1) SELECT * FROM src ORDER BY key"},
        {"cte-compound", "WITH src AS (SELECT key,Amount FROM %1 UNION ALL SELECT key,Amount+1000 FROM %1)"
                         " SELECT * FROM src ORDER BY key,Amount"},
        {"cte-aggregate", "WITH src AS (SELECT parent,sum(Amount) AS total FROM %1 GROUP BY parent)"
                          " SELECT * FROM src ORDER BY parent"},
        {"cte-window", "WITH src AS (SELECT key,sum(Amount) OVER (ORDER BY key) AS total FROM %1)"
                       " SELECT * FROM src ORDER BY key"},
        {"recursive-ordinary", "WITH RECURSIVE src AS (SELECT key,Amount FROM %1) SELECT * FROM src ORDER BY key"},
        {"recursive-seed", "WITH RECURSIVE seq(k) AS (SELECT key FROM %1 WHERE key=1 UNION ALL"
                           " SELECT k+1 FROM seq WHERE k<3) SELECT k FROM seq ORDER BY k"},
        {"recursive-step", "WITH RECURSIVE walk(k) AS (VALUES(1) UNION ALL"
                           " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k) SELECT k FROM walk ORDER BY k"},
        {"recursive-union", "WITH RECURSIVE walk(k) AS (SELECT key FROM %1 WHERE parent IS NULL UNION"
                            " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k) SELECT k FROM walk ORDER BY k"},
        {"recursive-chain", "WITH RECURSIVE src AS (SELECT * FROM %1), walk(k) AS"
                            " (SELECT key FROM src WHERE parent IS NULL UNION ALL"
                            " SELECT src.key FROM src JOIN walk w ON src.parent=w.k) SELECT k FROM walk ORDER BY k"},
        {"recursive-nested", "WITH result AS (WITH RECURSIVE walk(k) AS (VALUES(1) UNION ALL"
                             " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k) SELECT k FROM walk)"
                             " SELECT k FROM result ORDER BY k"},
        {"recursive-without-keyword", "WITH walk(k) AS (SELECT key FROM %1 WHERE parent IS NULL UNION ALL"
                                     " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k) SELECT k FROM walk ORDER BY k"},
        {"recursive-multiple-arms", "WITH RECURSIVE walk(k) AS (VALUES(1) UNION ALL"
                                   " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k AND ri.key=2 UNION ALL"
                                   " SELECT ri.key FROM %1 ri JOIN walk w ON ri.parent=w.k AND ri.key=3)"
                                   " SELECT k FROM walk ORDER BY k"}
    });
}

void QueryExecutorTest::implicitSelect()
{
    checkImplicitSelect();
}

void QueryExecutorTest::checkImplicitSelect(bool expectFallback)
{
    QFETCH(QString, sql);
    QFETCH(bool, connected);
    QFETCH(bool, sameNamedTable);
    if (sameNamedTable)
    {
        QVERIFY(execSql(db, "CREATE TABLE btable(key INTEGER PRIMARY KEY, parent INTEGER, Amount INTEGER)"));
        QVERIFY(execSql(db, "INSERT INTO btable VALUES(99,NULL,990)"));
    }
    if (connected)
    {
        QVERIFY(DBLIST->getByName("db2")->open());
        QVERIFY(DBLIST->getByName("db3")->open());
    }
    QCOMPARE(db->exec("PRAGMA database_list")->getAll().size(), 1);

    checkSelect(sql, false, expectFallback);
}

void QueryExecutorTest::checkSelect(const QString& sql, bool noMetaColumns, bool expectFallback)
{
    int attachmentCount = db->exec("PRAGMA database_list")->getAll().size();

    // Separate connection: the executor under test must perform its own implicit attachments.
    DbSqlite3 reference("reference", db->getPath());
    QVERIFY(reference.open());
    for (const QString& name : {QString("db2"), QString("db3")})
    {
        SqlQueryPtr attach = reference.exec("ATTACH DATABASE ? AS " + name, {DBLIST->getByName(name)->getPath()});
        QVERIFY2(!attach->isError(), qPrintable(attach->getErrorText()));
    }
    SqlQueryPtr native = reference.exec(sql, Db::Flag::PRELOAD);
    QVERIFY2(!native->isError(), qPrintable(native->getErrorText()));
    Rows expected = readRows(native);
    QStringList expectedColumns = native->getColumnNames();
    native.clear();
    QVERIFY(reference.close());

    QueryExecutor executor(db, sql);
    executor.setAsyncMode(false);
    executor.setPreloadResults(true);
    executor.setNoMetaColumns(noMetaColumns);
    executor.setResultsPerPage(-1);
    QSignalSpy failed(&executor, &QueryExecutor::executionFailed);
    QSignalSpy finished(&executor, &QueryExecutor::executionFinished);
    auto releaseResults = qScopeGuard([&] {
        finished.clear();
        executor.releaseResultsAndCleanup();
    });
    executor.exec();
    QVERIFY2(failed.isEmpty(), failed.isEmpty() ? "" : qPrintable(failed.first().at(1).toString()));
    QCOMPARE(executor.getEditionForbiddenGlobalReasons().contains(
                 QueryExecutor::EditionForbiddenReason::SMART_EXECUTION_FAILED), expectFallback);
    QCOMPARE(finished.size(), 1);
    QStringList resultColumns;
    QStringList displayNames;
    for (const auto& column : executor.getResultColumns())
    {
        resultColumns << (column->queryExecutorAlias.isEmpty() ? column->displayName : column->queryExecutorAlias);
        displayNames << column->displayName;
    }
    QCOMPARE(displayNames, expectedColumns);
    QCOMPARE(readRows(executor.getResults(), resultColumns), expected);
    QVERIFY(executor.countResults());
    QCOMPARE(executor.getTotalRowsReturned(), qint64(expected.size()));
    finished.clear();
    executor.releaseResultsAndCleanup();
    releaseResults.dismiss();
    QCOMPARE(db->exec("PRAGMA database_list")->getAll().size(), attachmentCount);
}

void QueryExecutorTest::cteControls_data()
{
    QTest::addColumn<QString>("sql");
    QTest::addColumn<int>("mode");
    QList<QPair<QString, QString>> queries = {
        {"view", "SELECT ri.Amount FROM db2.RegInfo ri ORDER BY ri.key"},
        {"forward-reference", "WITH final AS (SELECT key,Amount FROM src), src AS (SELECT * FROM db2.btable)"
                              " SELECT * FROM final ORDER BY key"},
        {"independent", "WITH left_side AS (SELECT key FROM db2.btable), right_side AS (SELECT key,Amount FROM db2.btable)"
                        " SELECT l.key,r.Amount FROM left_side l JOIN right_side r ON l.key=r.key ORDER BY l.key"},
        {"database-name", "WITH db2 AS (SELECT key,Amount FROM db2.btable) SELECT db2.key,db2.Amount FROM db2 ORDER BY key"},
        {"materialized", "WITH src AS MATERIALIZED (SELECT key,Amount FROM db2.btable) SELECT * FROM src ORDER BY key"},
        {"not-materialized", "WITH src AS NOT MATERIALIZED (SELECT key,Amount FROM db2.btable) SELECT * FROM src ORDER BY key"}
    };
    for (const auto& query : queries)
        for (int mode = 0; mode < 3; mode++)
        {
            QString tag = query.first + "/" + QStringList{"manual-attach", "no-metadata", "local-database"}.at(mode);
            QTest::newRow(qPrintable(tag)) << query.second << mode;
        }
}

void QueryExecutorTest::cteControls()
{
    QFETCH(QString, sql);
    QFETCH(int, mode);
    if (mode != 1)
    {
        SqlQueryPtr attach = db->exec("ATTACH DATABASE ? AS db2", {DBLIST->getByName("db2")->getPath()});
        QVERIFY2(!attach->isError(), qPrintable(attach->getErrorText()));
        attach.clear();
        if (mode == 2)
        {
            QVERIFY(execSql(db, "CREATE TABLE btable AS SELECT * FROM db2.btable"));
            QVERIFY(execSql(db, "CREATE VIEW RegInfo AS SELECT * FROM btable"));
            QVERIFY(execSql(db, "DETACH DATABASE db2"));
            sql.replace("db2.btable", "main.btable");
            sql.replace("db2.RegInfo", "main.RegInfo");
        }
    }
    checkSelect(sql, mode == 1);
}

void QueryExecutorTest::viewScopes_data()
{
    addQueryRows({
        {"star", "SELECT * FROM %1 ORDER BY key"},
        {"column", "SELECT ri.key,ri.Amount FROM %1 ri ORDER BY ri.key"},
        {"cte", "WITH src AS (SELECT key,Amount FROM %1) SELECT * FROM src ORDER BY key"}
    }, {"JoinedView", "ScopedView", "InView", "NestedView"});
}

void QueryExecutorTest::viewScopes()
{
    checkImplicitSelect();
}

void QueryExecutorTest::nestedRowIds_data()
{
    addQueryRows({
        {"self-join", "SELECT l.key AS left_key,r.key AS right_key FROM (SELECT * FROM (SELECT * FROM %1) src) l"
                      " JOIN (SELECT * FROM (SELECT * FROM %1) src) r ON r.key=l.key+1 ORDER BY l.key"},
        {"mixed-case", "SELECT l.key AS left_key,r.key AS right_key FROM (SELECT * FROM %1) L"
                        " JOIN (SELECT * FROM %1) R ON r.key=l.key+1 ORDER BY l.key"},
        {"two-databases", "SELECT l.key AS left_key,r.key AS right_key FROM (SELECT * FROM %1) l"
                           " JOIN (SELECT * FROM db3.btable) r ON r.key=l.key+1 ORDER BY l.key"}
    }, {"btable", "RegInfo", "NestedView"});
}

void QueryExecutorTest::nestedRowIds()
{
    QFETCH(QString, sql);
    checkImplicitSelect();
    if (QTest::currentTestFailed())
        return;

    QueryExecutor executor(db, sql);
    executor.setAsyncMode(false);
    executor.setPreloadResults(true);
    QSignalSpy failed(&executor, &QueryExecutor::executionFailed);
    auto releaseResults = qScopeGuard([&] {executor.releaseResultsAndCleanup();});
    executor.exec();
    QVERIFY2(failed.isEmpty(), failed.isEmpty() ? "" : qPrintable(failed.first().at(1).toString()));
    QVERIFY(!executor.getEditionForbiddenGlobalReasons().contains(QueryExecutor::EditionForbiddenReason::SMART_EXECUTION_FAILED));
    auto rowIds = executor.getRowIdResultColumns();
    QCOMPARE(rowIds.size(), 2);
    QHash<QString, QString> resultAliases;
    for (const auto& column : executor.getResultColumns())
        resultAliases[column->displayName] = column->queryExecutorAlias;

    auto rows = executor.getResults()->getAll();
    QVERIFY(!rows.isEmpty());
    for (const auto& row : rows)
        for (const auto& rowId : rowIds)
        {
            bool left = rowId->tableAlias.compare("l", Qt::CaseInsensitive) == 0;
            QVERIFY(left || rowId->tableAlias.compare("r", Qt::CaseInsensitive) == 0);
            QCOMPARE(rowId->queryExecutorAliasToColumn.size(), 1);
            QString rowIdAlias = *rowId->queryExecutorAliasToColumn.keyBegin();
            QString keyAlias = resultAliases.value(left ? "left_key" : "right_key");
            QVERIFY(!keyAlias.isEmpty());
            QCOMPARE(row->value(rowIdAlias).toLongLong(), row->value(keyAlias).toLongLong());
        }
}

void QueryExecutorTest::metadataFallback_data()
{
    addQueryRows({
        {"direct", "SELECT key,Amount FROM %1 ORDER BY key"},
        {"cte", "WITH final AS (SELECT * FROM src), src AS (SELECT key,Amount FROM %1)"
                " SELECT * FROM final ORDER BY key"},
        {"materialized", "WITH src AS MATERIALIZED (SELECT key,Amount FROM %1) SELECT * FROM src ORDER BY key"}
    });
}

void QueryExecutorTest::metadataFallback()
{
    QFETCH(QString, sql);
    QFETCH(bool, connected);
    QFETCH(bool, sameNamedTable);
    if (connected)
        QVERIFY(DBLIST->getByName("db2")->open());
    if (sameNamedTable)
    {
        QVERIFY(execSql(db, "CREATE TABLE btable(key INTEGER PRIMARY KEY, parent INTEGER, Amount INTEGER)"));
        QVERIFY(execSql(db, "INSERT INTO btable VALUES(99,NULL,990)"));
    }
    FailMetadataStep step;
    QueryExecutor::registerStep(QueryExecutor::AFTER_REPLACED_COLUMNS, &step);
    auto deregisterStep = qScopeGuard([&] {QueryExecutor::deregisterStep(QueryExecutor::AFTER_REPLACED_COLUMNS, &step);});
    checkSelect(sql, false, true);
}

void QueryExecutorTest::implicitCteDml_data()
{
    addQueryRows({
        {"cte-insert", "WITH src AS (SELECT key,Amount FROM %1) INSERT INTO destination SELECT * FROM src"},
        {"cte-update", "WITH src AS (SELECT key,Amount FROM %1), final AS (SELECT * FROM src)"
                       " UPDATE destination SET Amount=(SELECT Amount FROM final WHERE final.key=destination.key)"
                       " WHERE key IN (SELECT key FROM final)"},
        {"cte-delete", "WITH src AS (WITH inner_src AS (SELECT key FROM %1) SELECT * FROM inner_src)"
                       " DELETE FROM destination WHERE key IN (SELECT key FROM src)"}
    });
}

void QueryExecutorTest::implicitCteDml()
{
    QFETCH(QString, sql);
    QFETCH(bool, connected);
    QFETCH(bool, sameNamedTable);
    if (sameNamedTable)
    {
        QVERIFY(execSql(db, "CREATE TABLE btable(key INTEGER PRIMARY KEY, parent INTEGER, Amount INTEGER)"));
        QVERIFY(execSql(db, "INSERT INTO btable VALUES(99,NULL,990)"));
    }
    if (connected)
        QVERIFY(DBLIST->getByName("db2")->open());

    if (!sql.contains("INSERT INTO"))
        QVERIFY(execSql(db, "INSERT INTO destination SELECT * FROM atable"));

    // Validate SQLite syntax and expected changes, then roll back the disposable reference execution.
    SqlQueryPtr attach = db->exec("ATTACH DATABASE ? AS db2", {DBLIST->getByName("db2")->getPath()});
    QVERIFY2(!attach->isError(), qPrintable(attach->getErrorText()));
    attach.clear();
    QVERIFY(execSql(db, "SAVEPOINT reference"));
    QVERIFY(execSql(db, sql));
    Rows expected = readRows(db->exec("SELECT * FROM destination ORDER BY key", Db::Flag::PRELOAD));
    QVERIFY(execSql(db, "ROLLBACK TO reference"));
    QVERIFY(execSql(db, "RELEASE reference"));
    QVERIFY(execSql(db, "DETACH DATABASE db2"));
    QCOMPARE(db->exec("PRAGMA database_list")->getAll().size(), 1);

    QueryExecutor executor(db, sql);
    executor.setAsyncMode(false);
    executor.setPreloadResults(true);
    QSignalSpy failed(&executor, &QueryExecutor::executionFailed);
    auto releaseResults = qScopeGuard([&] {executor.releaseResultsAndCleanup();});
    executor.exec();
    QVERIFY2(failed.isEmpty(), failed.isEmpty() ? "" : qPrintable(failed.first().at(1).toString()));
    QCOMPARE(readRows(db->exec("SELECT * FROM destination ORDER BY key", Db::Flag::PRELOAD)), expected);
    executor.releaseResultsAndCleanup();
    releaseResults.dismiss();
    QCOMPARE(db->exec("PRAGMA database_list")->getAll().size(), 1);
}

QTEST_GUILESS_MAIN(QueryExecutorTest)

#include "tst_queryexecutortest.moc"
