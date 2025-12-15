#include "dbattacherimpl.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include "services/dbmanager.h"
#include "parser/parser.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include <QDebug>

DbAttacherImpl::DbAttacherImpl(Db* db)
{
    this->db = db;
}

bool DbAttacherImpl::attachDatabases(const QString& query)
{
    Parser parser;
    if (!parser.parse(query))
        return false;

    queries = parser.getQueries();
    return attachDatabases();
}

bool DbAttacherImpl::attachDatabases(const QList<SqliteQueryPtr>& queries)
{
    this->queries = queries;
    return attachDatabases();
}

bool DbAttacherImpl::attachDatabases(SqliteQueryPtr query)
{
    queries.clear();
    queries << query;
    return attachDatabases();
}

void DbAttacherImpl::detachDatabases()
{
    detachAttached();
}

bool DbAttacherImpl::attachDatabases()
{
    dbNameToAttach.clear();
    prepareNameToDbMap();

    BiStrHash nativePathToAttachName = getNativePathToAttachName();

    TokenList dbTokens = getDbTokens() | FILTER(t, {
        return !nativePathToAttachName.containsRight(t->value, Qt::CaseInsensitive);
    });

    StrHash<TokenList> groupedDbTokens = groupDbTokens(dbTokens);
    for (QString& nativeName : nativePathToAttachName.rightValues())
        groupedDbTokens.remove(nativeName, Qt::CaseInsensitive);

    if (!attachAllDbs(groupedDbTokens))
        return false;

    QHash<TokenPtr,TokenPtr> tokenMapping = getTokenMapping(dbTokens);
    replaceTokensInQueries(tokenMapping);

    return true;
}

TokenList DbAttacherImpl::getDbTokens()
{
    TokenList dbTokens;
    for (SqliteQueryPtr& query : queries)
        dbTokens += query->getContextDatabaseTokens();

    return dbTokens;
}

void DbAttacherImpl::detachAttached()
{
    for (const QString& dbName : dbNameToAttach.leftValues())
        db->detach(nameToDbMap[dbName]);

    dbNameToAttach.clear();
    nameToDbMap.clear();
}

void DbAttacherImpl::prepareNameToDbMap()
{
    for (Db* db : DBLIST->getValidDbList())
        nameToDbMap[db->getName()] = db;
}

StrHash<TokenList> DbAttacherImpl::groupDbTokens(const TokenList& dbTokens)
{
    // Filter out tokens of unknown databases and group results by name
    StrHash<TokenList> groupedDbTokens;
    QString strippedName;
    for (const TokenPtr& token : dbTokens)
    {
        strippedName = stripObjName(token->value);
        if (!nameToDbMap.contains(strippedName, Qt::CaseInsensitive))
            continue;

        groupedDbTokens[strippedName] += token;
    }
    return groupedDbTokens;
}

bool DbAttacherImpl::attachAllDbs(const StrHash<TokenList>& groupedDbTokens)
{
    QString attachName;
    for (const QString& dbName : groupedDbTokens.keys())
    {
        if (dbName.toLower() == "main")
        {
            mainDbNameUsed = true;
            continue;
        }

        attachName = db->attach(nameToDbMap[dbName]);
        if (attachName.isNull())
        {
            notifyError(QObject::tr("Could not attach database %1: %2").arg(dbName, db->getErrorText()));
            detachAttached();
            return false;
        }

        dbNameToAttach.insert(dbName, attachName);
    }
    return true;
}

QHash<TokenPtr, TokenPtr> DbAttacherImpl::getTokenMapping(const TokenList& dbTokens)
{
    QHash<TokenPtr, TokenPtr> tokenMapping;
    QString strippedName;
    TokenPtr dstToken;
    for (const TokenPtr& srcToken : dbTokens)
    {
        strippedName = stripObjName(srcToken->value);
        if (strippedName.compare("main", Qt::CaseInsensitive) == 0 ||
                strippedName.compare("temp", Qt::CaseInsensitive) == 0)
            continue;

        if (!dbNameToAttach.containsLeft(strippedName, Qt::CaseInsensitive))
        {
            qCritical() << "DB token to be replaced, but it's not on nameToAlias map! Token value:" << strippedName
                        << "and nameToAlias map has keys:" << dbNameToAttach.leftValues();
            continue;
        }
        dstToken = TokenPtr::create(dbNameToAttach.valueByLeft(strippedName, Qt::CaseInsensitive));
        tokenMapping[srcToken] = dstToken;
    }
    return tokenMapping;
}

void DbAttacherImpl::replaceTokensInQueries(const QHash<TokenPtr, TokenPtr>& tokenMapping)
{
    int idx;
    QHashIterator<TokenPtr,TokenPtr> it(tokenMapping);
    while (it.hasNext())
    {
        it.next();
        for (SqliteQueryPtr& query : queries)
        {
            idx = query->tokens.indexOf(it.key());
            if (idx < 0)
                continue; // token not in this query, most likely in other query

            query->tokens.replace(idx, it.value());
        }
    }
}

BiStrHash DbAttacherImpl::getNativePathToAttachName() const
{
    BiStrHash nativePathToAttachName;
    SqlQueryPtr res = db->exec("PRAGMA database_list");
    if (res->isError())
    {
        qCritical() << "Failed to query existing database attachmens with PRAGMA database_list."
                    << res->getErrorCode()
                    << res->getErrorText();
        return nativePathToAttachName;
    }

    QList<SqlResultsRowPtr> rows = res->getAll();
    for (SqlResultsRowPtr& row : rows)
    {
        QString attName = row->value("name").toString();
        if (attName == "main" || attName == "temp")
            continue;

        QString path = row->value("file").toString();
        if (path.isEmpty())
            continue;

        nativePathToAttachName.insert(path, attName);
    }
    return nativePathToAttachName;
}

bool DbAttacherImpl::getMainDbNameUsed() const
{
    return mainDbNameUsed;
}

BiStrHash DbAttacherImpl::getDbNameToAttach() const
{
    return dbNameToAttach;
}

QString DbAttacherImpl::getQuery() const
{
    QStringList queryStrings;
    for (const SqliteQueryPtr& query : queries)
        queryStrings << query->detokenize();

    return queryStrings.join(";");
}

DbAttacher* DbAttacherDefaultFactory::create(Db* db)
{
    return new DbAttacherImpl(db);
}
