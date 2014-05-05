#include "dbversionconverter.h"
#include "schemaresolver.h"
#include "common/global.h"
#include "parser/ast/sqlitealtertable.h"
#include "parser/ast/sqliteanalyze.h"
#include "parser/ast/sqliteattach.h"
#include "parser/ast/sqlitebegintrans.h"
#include "parser/ast/sqlitecommittrans.h"
#include "parser/ast/sqlitecopy.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqlitedetach.h"
#include "parser/ast/sqlitedropindex.h"
#include "parser/ast/sqlitedroptable.h"
#include "parser/ast/sqlitedroptrigger.h"
#include "parser/ast/sqlitedropview.h"
#include "parser/ast/sqliteemptyquery.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqlitepragma.h"
#include "parser/ast/sqlitereindex.h"
#include "parser/ast/sqliterelease.h"
#include "parser/ast/sqliterollback.h"
#include "parser/ast/sqlitesavepoint.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqlitevacuum.h"
#include <QDebug>

DbVersionConverter::DbVersionConverter()
{
}

DbVersionConverter::~DbVersionConverter()
{
    safe_delete(resolver);
}

void DbVersionConverter::convertToVersion2(Db* db)
{
    reset();
    this->db = db;
    targetDialect = Dialect::Sqlite2;
    convertDb();
}

void DbVersionConverter::convertToVersion3(Db* db)
{
    reset();
    this->db = db;
    targetDialect = Dialect::Sqlite3;
    convertDb();
}

QString DbVersionConverter::convertToVersion2(const QString& sql)
{
    QStringList result;
    for (SqliteQueryPtr query : parse(sql, Dialect::Sqlite3))
        result << convertToVersion2(query)->detokenize();

    return result.join("\n");
}

QString DbVersionConverter::convertToVersion3(const QString& sql)
{
    QStringList result;
    for (SqliteQueryPtr query : parse(sql, Dialect::Sqlite2))
        result << convertToVersion3(query)->detokenize();

    return result.join("\n");
}

SqliteQueryPtr DbVersionConverter::convertToVersion2(SqliteQueryPtr query)
{
    SqliteQueryPtr newQuery;
    switch (query->queryType)
    {
        case SqliteQueryType::AlterTable:
            errors << QObject::tr("SQLite 2 does not support 'ALTER TABLE' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Analyze:
            errors << QObject::tr("SQLite 2 does not support 'ANAYLZE' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Attach:
            newQuery = copyQuery<SqliteAttach>(query);
            break;
        case SqliteQueryType::BeginTrans:
            newQuery = copyQuery<SqliteBeginTrans>(query);
            break;
        case SqliteQueryType::CommitTrans:
            newQuery = copyQuery<SqliteCommitTrans>(query);
            break;
        case SqliteQueryType::Copy:
            qWarning() << "COPY query passed to DbVersionConverter::convertToVersion2(). SQLite3 query should not have COPY statement.";
            newQuery = copyQuery<SqliteCopy>(query);
            break;
        case SqliteQueryType::CreateIndex:
            newQuery = copyQuery<SqliteCreateIndex>(query);
            if (!modifyCreateIndexForVersion2(newQuery.dynamicCast<SqliteCreateIndex>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::CreateTable:
            newQuery = copyQuery<SqliteCreateTable>(query);
            if (!modifyCreateTableForVersion2(newQuery.dynamicCast<SqliteCreateTable>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::CreateTrigger:
            newQuery = copyQuery<SqliteCreateTrigger>(query);
            if (!modifyCreateTriggerForVersion2(newQuery.dynamicCast<SqliteCreateTrigger>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::CreateView:
            newQuery = copyQuery<SqliteCreateView>(query);
            if (!modifyCreateViewForVersion2(newQuery.dynamicCast<SqliteCreateView>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::CreateVirtualTable:
            errors << QObject::tr("SQLite 2 does not support 'CREATE VIRTUAL TABLE' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Delete:
            newQuery = copyQuery<SqliteDelete>(query);
            if (!modifyDeleteForVersion2(newQuery.dynamicCast<SqliteDelete>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::Detach:
            newQuery = copyQuery<SqliteDetach>(query);
            break;
        case SqliteQueryType::DropIndex:
            newQuery = copyQuery<SqliteDropIndex>(query);
            break;
        case SqliteQueryType::DropTable:
            newQuery = copyQuery<SqliteDropTable>(query);
            break;
        case SqliteQueryType::DropTrigger:
            newQuery = copyQuery<SqliteDropTrigger>(query);
            break;
        case SqliteQueryType::DropView:
            newQuery = copyQuery<SqliteDropView>(query);
            break;
        case SqliteQueryType::Insert:
            newQuery = copyQuery<SqliteInsert>(query);
            if (!modifyInsertForVersion2(newQuery.dynamicCast<SqliteInsert>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::Pragma:
            newQuery = copyQuery<SqlitePragma>(query);
            break;
        case SqliteQueryType::Reindex:
            errors << QObject::tr("SQLite 2 does not support 'REINDEX' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Release:
            errors << QObject::tr("SQLite 2 does not support 'RELEASE' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Rollback:
            newQuery = copyQuery<SqliteRollback>(query);
            break;
        case SqliteQueryType::Savepoint:
            errors << QObject::tr("SQLite 2 does not support 'SAVEPOINT' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::Select:
        {
            newQuery = copyQuery<SqliteSelect>(query);
            if (!modifySelectForVersion2(newQuery.dynamicCast<SqliteSelect>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        }
        case SqliteQueryType::Update:
            newQuery = copyQuery<SqliteUpdate>(query);
            if (!modifyUpdateForVersion2(newQuery.dynamicCast<SqliteUpdate>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::Vacuum:
            newQuery = copyQuery<SqliteVacuum>(query);
            break;
        case SqliteQueryType::UNDEFINED:
            qWarning() << "UNDEFINED query type passed to DbVersionConverter::convertToVersion2().";
            newQuery = SqliteEmptyQueryPtr::create();
            break;
        case SqliteQueryType::EMPTY:
            newQuery = copyQuery<SqliteEmptyQuery>(query);
            break;
    }

    if (!newQuery)
    {
        qCritical() << "Query type not matched in DbVersionConverter::convertToVersion2():" << static_cast<int>(query->queryType);
        return SqliteQueryPtr();
    }

    if (newQuery->queryType != SqliteQueryType::EMPTY)
    {
        newQuery->setSqliteDialect(Dialect::Sqlite2);
        newQueries << newQuery;
    }

    newQuery->rebuildTokens();
    return newQuery;
}

SqliteQueryPtr DbVersionConverter::convertToVersion3(SqliteQueryPtr query)
{
    SqliteQueryPtr newQuery;
    switch (query->queryType)
    {
        case SqliteQueryType::AlterTable:
            newQuery = copyQuery<SqliteAlterTable>(query);
            qWarning() << "ALTER TABLE query passed to DbVersionConverter::convertToVersion3(). SQLite2 query should not have ALTER TABLE statement.";
            break;
        case SqliteQueryType::Analyze:
            newQuery = copyQuery<SqliteAnalyze>(query);
            qWarning() << "ANALYZE query passed to DbVersionConverter::convertToVersion3(). SQLite2 query should not have ANALYZE statement.";
            break;
        case SqliteQueryType::Attach:
            newQuery = copyQuery<SqliteAttach>(query);
            break;
        case SqliteQueryType::BeginTrans:
            newQuery = copyQuery<SqliteBeginTrans>(query);
            if (!modifyBeginTransForVersion3(newQuery.dynamicCast<SqliteBeginTrans>().data()))
            {
                newQuery = SqliteEmptyQueryPtr::create();
                storeErrorDiff(query.data());
            }
            break;
        case SqliteQueryType::CommitTrans:
            newQuery = copyQuery<SqliteCommitTrans>(query);
            break;
        case SqliteQueryType::Copy:
            errors << QObject::tr("SQLite 3 does not support 'COPY' statement.");
            newQuery = SqliteEmptyQueryPtr::create();
            storeErrorDiff(query.data());
            break;
        case SqliteQueryType::CreateIndex:
            newQuery = copyQuery<SqliteCreateIndex>(query);
            break;
        case SqliteQueryType::CreateTable:
            newQuery = copyQuery<SqliteCreateTable>(query);
            break;
        case SqliteQueryType::CreateTrigger:
            newQuery = copyQuery<SqliteCreateTrigger>(query);
            break;
        case SqliteQueryType::CreateView:
            newQuery = copyQuery<SqliteCreateView>(query);
            break;
        case SqliteQueryType::CreateVirtualTable:
            newQuery = copyQuery<SqliteCreateVirtualTable>(query);
            break;
        case SqliteQueryType::Delete:
            newQuery = copyQuery<SqliteDelete>(query);
            break;
        case SqliteQueryType::Detach:
            newQuery = copyQuery<SqliteDetach>(query);
            break;
        case SqliteQueryType::DropIndex:
            newQuery = copyQuery<SqliteDropIndex>(query);
            break;
        case SqliteQueryType::DropTable:
            newQuery = copyQuery<SqliteDropTable>(query);
            break;
        case SqliteQueryType::DropTrigger:
            newQuery = copyQuery<SqliteDropTrigger>(query);
            break;
        case SqliteQueryType::DropView:
            newQuery = copyQuery<SqliteDropView>(query);
            break;
        case SqliteQueryType::Insert:
            newQuery = copyQuery<SqliteInsert>(query);
            break;
        case SqliteQueryType::Pragma:
            newQuery = copyQuery<SqlitePragma>(query);
            break;
        case SqliteQueryType::Reindex:
            newQuery = copyQuery<SqliteReindex>(query);
            qWarning() << "REINDEX query passed to DbVersionConverter::convertToVersion3(). SQLite2 query should not have REINDEX statement.";
            break;
        case SqliteQueryType::Release:
            newQuery = copyQuery<SqliteRelease>(query);
            qWarning() << "RELEASE query passed to DbVersionConverter::convertToVersion3(). SQLite2 query should not have RELEASE statement.";
            break;
        case SqliteQueryType::Rollback:
            newQuery = copyQuery<SqliteRollback>(query);
            break;
        case SqliteQueryType::Savepoint:
            newQuery = copyQuery<SqliteSavepoint>(query);
            qWarning() << "SAVEPOINT query passed to DbVersionConverter::convertToVersion3(). SQLite2 query should not have SAVEPOINT statement.";
            break;
        case SqliteQueryType::Select:
            newQuery = copyQuery<SqliteSelect>(query);
            break;
        case SqliteQueryType::Update:
            newQuery = copyQuery<SqliteUpdate>(query);
            break;
        case SqliteQueryType::Vacuum:
            newQuery = copyQuery<SqliteVacuum>(query);
            break;
        case SqliteQueryType::UNDEFINED:
            qWarning() << "UNDEFINED query type passed to DbVersionConverter::convertToVersion3().";
        case SqliteQueryType::EMPTY:
            newQuery = copyQuery<SqliteEmptyQuery>(query);
            break;
    }

    if (!newQuery)
    {
        qCritical() << "Query type not matched in DbVersionConverter::convertToVersion3():" << static_cast<int>(query->queryType);
        return SqliteQueryPtr();
    }

    if (newQuery->queryType != SqliteQueryType::EMPTY)
    {
        newQuery->setSqliteDialect(Dialect::Sqlite2);
        newQueries << newQuery;
    }
    return newQuery;
}

QList<SqliteQueryPtr> DbVersionConverter::parse(const QString& sql, Dialect dialect)
{
    Parser parser(dialect);
    if (!parser.parse(sql))
    {
        errors << QObject::tr("Could not parse statement: %1\nError details: %2").arg(sql, parser.getErrorString());
        return QList<SqliteQueryPtr>();
    }

    return parser.getQueries();
}

bool DbVersionConverter::modifySelectForVersion2(SqliteSelect* select)
{
    if (select->with)
    {
        errors << QObject::tr("SQLite 2 does not support the 'WITH' clause. Cannot convert '%1' statement with that clause.").arg("SELECT");
        return false;
    }

    QString sql1 = getSqlForDiff(select);

    for (SqliteSelect::Core* core : select->coreSelects)
    {
        if (core->valuesMode)
            core->valuesMode = false;
    }

    if (!modifyAllIndexedColumnsForVersion2(select))
        return false;

    if (!modifyAllExprsForVersion2(select))
        return false;

    storeDiff(sql1, select);
    return true;
}

bool DbVersionConverter::modifyDeleteForVersion2(SqliteDelete* del)
{
    if (del->with)
    {
        errors << QObject::tr("SQLite 2 does not support the 'WITH' clause. Cannot convert '%1' statement with that clause.").arg("DELETE");
        return false;
    }

    QString sql1 = getSqlForDiff(del);

    del->indexedBy = QString::null;
    del->indexedByKw = false;
    del->notIndexedKw = false;

    if (!modifyAllExprsForVersion2(del))
        return false;

    storeDiff(sql1, del);
    return true;
}

bool DbVersionConverter::modifyInsertForVersion2(SqliteInsert* insert)
{
    if (insert->with)
    {
        errors << QObject::tr("SQLite 2 does not support the 'WITH' clause. Cannot convert '%1' statement with that clause.").arg("INSERT");
        return false;
    }

    if (insert->defaultValuesKw)
    {
        errors << QObject::tr("SQLite 2 does not support the 'DEFAULT VALUES' clause in the 'INSERT' clause.");
        return false;
    }

    if (!insert->select)
    {
        qCritical() << "No SELECT substatement in INSERT when converting from SQLite 3 to 2.";
        return false;
    }

    QString sql1 = getSqlForDiff(insert);

    // Modifying SELECT deals with "VALUES" completely.
    if (!modifySelectForVersion2(insert->select))
        return false;

    if (!modifyAllExprsForVersion2(insert))
        return false;

    storeDiff(sql1, insert);
    return true;
}

bool DbVersionConverter::modifyUpdateForVersion2(SqliteUpdate* update)
{
    if (update->with)
    {
        errors << QObject::tr("SQLite 2 does not support the 'WITH' clause. Cannot convert '%1' statement with that clause.").arg("UPDATE");
        return false;
    }

    QString sql1 = getSqlForDiff(update);

    if (!modifyAllExprsForVersion2(update))
        return false;

    update->indexedBy = QString::null;
    update->indexedByKw = false;
    update->notIndexedKw = false;

    storeDiff(sql1, update);
    return true;
}

bool DbVersionConverter::modifyCreateTableForVersion2(SqliteCreateTable* createTable)
{
    QString sql1 = getSqlForDiff(createTable);

    if (!createTable->database.isNull())
        createTable->database = QString::null;

    // Subselect
    if (createTable->select)
    {
        if (!modifySelectForVersion2(createTable->select))
            return false;
    }

    // Table constraints
    QMutableListIterator<SqliteCreateTable::Constraint*> tableConstrIt(createTable->constraints);
    while (tableConstrIt.hasNext())
    {
        tableConstrIt.next();
        if (tableConstrIt.value()->type == SqliteCreateTable::Constraint::PRIMARY_KEY)
            tableConstrIt.value()->autoincrKw = false;
    }

    // Column constraints
    QMutableListIterator<SqliteCreateTable::Column*> tableColIt(createTable->columns);
    while (tableColIt.hasNext())
    {
        tableColIt.next();
        QMutableListIterator<SqliteCreateTable::Column::Constraint*> tableColConstrIt(tableColIt.value()->constraints);
        while (tableColConstrIt.hasNext())
        {
            tableColConstrIt.next();
            switch (tableColConstrIt.value()->type)
            {
                case SqliteCreateTable::Column::Constraint::NAME_ONLY:
                {
                    delete tableColConstrIt.value();
                    tableColConstrIt.remove();
                    break;
                }
                case SqliteCreateTable::Column::Constraint::DEFAULT:
                {
                    if (!tableColConstrIt.value()->ctime.isNull())
                    {
                        delete tableColConstrIt.value();
                        tableColConstrIt.remove();
                    }
                    else
                        tableColConstrIt.value()->name = QString::null;

                    break;
                }
                case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
                {
                    tableColConstrIt.value()->autoincrKw = false;
                    break;
                }
                case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
                {
                    QMutableListIterator<SqliteForeignKey::Condition*> condIt(tableColConstrIt.value()->foreignKey->conditions);
                    while (condIt.hasNext())
                    {
                        condIt.next();
                        if (condIt.value()->reaction == SqliteForeignKey::Condition::NO_ACTION
                                && condIt.value()->action != SqliteForeignKey::Condition::MATCH) // SQLite 2 has no "NO ACTION"
                        {
                            condIt.remove();
                        }
                    }
                    break;
                }
                case SqliteCreateTable::Column::Constraint::NOT_NULL:
                case SqliteCreateTable::Column::Constraint::UNIQUE:
                case SqliteCreateTable::Column::Constraint::CHECK:
                case SqliteCreateTable::Column::Constraint::COLLATE:
                case SqliteCreateTable::Column::Constraint::NULL_:
                case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
                    break;
            }
        }
    }

    if (!modifyAllIndexedColumnsForVersion2(createTable))
        return false;

    // WITHOUT ROWID
    if (!createTable->withOutRowId.isNull())
        createTable->withOutRowId = QString::null;

    storeDiff(sql1, createTable);
    return true;
}

bool DbVersionConverter::modifyCreateTriggerForVersion2(SqliteCreateTrigger* createTrigger)
{
    QString sql1 = getSqlForDiff(createTrigger);

    if (!createTrigger->database.isNull())
        createTrigger->database = QString::null;

    for (SqliteQuery* query : createTrigger->queries)
    {
        switch (query->queryType)
        {
            case SqliteQueryType::Delete:
            {
                if (!modifyDeleteForVersion2(dynamic_cast<SqliteDelete*>(query)))
                    return false;

                break;
            }
            case SqliteQueryType::Update:
            {
                if (!modifyUpdateForVersion2(dynamic_cast<SqliteUpdate*>(query)))
                    return false;

                break;
            }
            case SqliteQueryType::Insert:
            {
                if (!modifyInsertForVersion2(dynamic_cast<SqliteInsert*>(query)))
                    return false;

                break;
            }
            case SqliteQueryType::Select:
            {
                if (!modifySelectForVersion2(dynamic_cast<SqliteSelect*>(query)))
                    return false;

                break;
            }
            default:
                qWarning() << "Unexpected query type in trigger:" << sqliteQueryTypeToString(query->queryType);
                break;
        }
    }

    storeDiff(sql1, createTrigger);
    return true;
}

bool DbVersionConverter::modifyCreateIndexForVersion2(SqliteCreateIndex* createIndex)
{
    QString sql1 = getSqlForDiff(createIndex);

    if (!createIndex->database.isNull())
        createIndex->database = QString::null;

    if (createIndex->where)
    {
        delete createIndex->where;
        createIndex->where = nullptr;
    }

    if (!modifyAllIndexedColumnsForVersion2(createIndex->indexedColumns))
        return false;

    storeDiff(sql1, createIndex);
    return true;
}

bool DbVersionConverter::modifyCreateViewForVersion2(SqliteCreateView* createView)
{
    QString sql1 = getSqlForDiff(createView);

    if (!createView->database.isNull())
        createView->database = QString::null;

    if (!modifySelectForVersion2(createView->select))
        return false;

    storeDiff(sql1, createView);
    return true;
}

bool DbVersionConverter::modifyAllExprsForVersion2(SqliteStatement* stmt)
{
    QList<SqliteExpr*> allExprs = stmt->getAllTypedStatements<SqliteExpr>();
    for (SqliteExpr* expr : allExprs)
    {
        if (!modifySingleExprForVersion2(expr))
            return false;
    }
    return true;
}

bool DbVersionConverter::modifySingleExprForVersion2(SqliteExpr* expr)
{
    switch (expr->mode)
    {
        case SqliteExpr::Mode::null:
        case SqliteExpr::Mode::LITERAL_VALUE:
        case SqliteExpr::Mode::BIND_PARAM:
        case SqliteExpr::Mode::ID:
        case SqliteExpr::Mode::UNARY_OP:
        case SqliteExpr::Mode::BINARY_OP:
        case SqliteExpr::Mode::FUNCTION:
        case SqliteExpr::Mode::SUB_EXPR:
        case SqliteExpr::Mode::LIKE:
        case SqliteExpr::Mode::NULL_:
        case SqliteExpr::Mode::NOTNULL:
        case SqliteExpr::Mode::IS:
        case SqliteExpr::Mode::BETWEEN:
        case SqliteExpr::Mode::CASE:
        case SqliteExpr::Mode::RAISE:
            break;
        case SqliteExpr::Mode::CTIME:
            errors << QObject::tr("SQLite 2 does not support current date or time clauses in expressions.");
            return false;
        case SqliteExpr::Mode::IN:
        case SqliteExpr::Mode::SUB_SELECT:
        {
            if (!modifySelectForVersion2(expr->select))
                return false;

            break;
        }
        case SqliteExpr::Mode::CAST:
            errors << QObject::tr("SQLite 2 does not support 'CAST' clause in expressions.");
            return false;
        case SqliteExpr::Mode::EXISTS:
            errors << QObject::tr("SQLite 2 does not support 'EXISTS' clause in expressions.");
            return false;
        case SqliteExpr::Mode::COLLATE:
        {
            if (dynamic_cast<SqliteOrderBy*>(expr->parentStatement()))
            {
                // This is the only case when SQLite2 parser puts this mode into expression, that is for sortorder
                break;
            }
            else
            {
                errors << QObject::tr("SQLite 2 does not support 'COLLATE' clause in expressions.");
                return false;
            }
        }
    }
    return true;
}

bool DbVersionConverter::modifyAllIndexedColumnsForVersion2(SqliteStatement* stmt)
{
    QList<SqliteIndexedColumn*> columns = stmt->getAllTypedStatements<SqliteIndexedColumn>();
    return modifyAllIndexedColumnsForVersion2(columns);

}

bool DbVersionConverter::modifyAllIndexedColumnsForVersion2(const QList<SqliteIndexedColumn*> columns)
{
    for (SqliteIndexedColumn* idxCol : columns)
    {
        if (!modifySingleIndexedColumnForVersion2(idxCol))
            return false;
    }
    return true;
}

bool DbVersionConverter::modifySingleIndexedColumnForVersion2(SqliteIndexedColumn* idxCol)
{
    if (!idxCol->collate.isNull())
        idxCol->collate = QString::null;

    return true;
}

bool DbVersionConverter::modifyBeginTransForVersion3(SqliteBeginTrans* begin)
{
    QString sql1 = getSqlForDiff(begin);
    begin->onConflict = SqliteConflictAlgo::null;
    storeDiff(sql1, begin);
    return true;
}

bool DbVersionConverter::modifyCreateTableForVersion3(SqliteCreateTable* createTable)
{
    QString sql1 = getSqlForDiff(createTable);

    // Column constraints
    QMutableListIterator<SqliteCreateTable::Column*> tableColIt(createTable->columns);
    while (tableColIt.hasNext())
    {
        tableColIt.next();
        QMutableListIterator<SqliteCreateTable::Column::Constraint*> tableColConstrIt(tableColIt.value()->constraints);
        while (tableColConstrIt.hasNext())
        {
            tableColConstrIt.next();
            switch (tableColConstrIt.value()->type)
            {
                case SqliteCreateTable::Column::Constraint::CHECK:
                {
                    tableColConstrIt.value()->onConflict = SqliteConflictAlgo::null;
                    break;
                }
                case SqliteCreateTable::Column::Constraint::NAME_ONLY:
                case SqliteCreateTable::Column::Constraint::DEFAULT:
                case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
                case SqliteCreateTable::Column::Constraint::NOT_NULL:
                case SqliteCreateTable::Column::Constraint::UNIQUE:
                case SqliteCreateTable::Column::Constraint::COLLATE:
                case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
                case SqliteCreateTable::Column::Constraint::NULL_:
                case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
                    break;
            }
        }
    }

    storeDiff(sql1, createTable);
    return true;
}

QString DbVersionConverter::getSqlForDiff(SqliteStatement* stmt)
{
    stmt->rebuildTokens();
    return stmt->detokenize();
}

void DbVersionConverter::storeDiff(const QString& sql1, SqliteStatement* stmt)
{
    stmt->rebuildTokens();
    QString sql2 = stmt->detokenize();
    if (sql1 != sql2)
        diffList << QPair<QString,QString>(sql1, sql2);
}

void DbVersionConverter::storeErrorDiff(SqliteStatement* stmt)
{
    stmt->rebuildTokens();
    diffList << QPair<QString,QString>(stmt->detokenize(), "");
}

void DbVersionConverter::reset()
{
    db = nullptr;
    targetDialect = Dialect::Sqlite3;
    diffList.clear();
    newQueries.clear();
    errors.clear();
    safe_delete(resolver);
}

const QList<QPair<QString, QString> >& DbVersionConverter::getDiffList() const
{
    return diffList;
}

const QStringList& DbVersionConverter::getErrors() const
{
    return errors;
}

const QList<SqliteQueryPtr>&DbVersionConverter::getConverted() const
{
    return newQueries;
}

QStringList DbVersionConverter::getConvertedSqls() const
{
    QStringList sqls;
    for (SqliteQueryPtr query : newQueries)
        sqls << query->detokenize();

    return sqls;
}

void DbVersionConverter::convertDb()
{
    resolver = new SchemaResolver(db);
    QHash<QString, SqliteQueryPtr> parsedObjects = resolver->getAllParsedObjects();
    for (SqliteQueryPtr query : parsedObjects.values())
    {
        switch (targetDialect)
        {
            case Dialect::Sqlite2:
                convertToVersion2(query);
                break;
            case Dialect::Sqlite3:
                convertToVersion3(query);
                break;
        }
    }
}
