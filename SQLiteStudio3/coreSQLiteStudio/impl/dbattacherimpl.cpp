#include "dbattacherimpl.h"
#include "db/db.h"
#include "services/dbmanager.h"
#include "parser/parser.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include <QDebug>

DbAttacherImpl::DbAttacherImpl(Db* db)
{
    this->db = db;
    dialect = db->getDialect();
}

bool DbAttacherImpl::attachDatabases(const QString& query)
{
    Parser parser(dialect);
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

    TokenList dbTokens = getDbTokens();
    QHash<QString,TokenList> groupedDbTokens = groupDbTokens(dbTokens);

    if (!attachAllDbs(groupedDbTokens))
        return false;

    QHash<TokenPtr,TokenPtr> tokenMapping = getTokenMapping(dbTokens);
    replaceTokensInQueries(tokenMapping);

    return true;
}

TokenList DbAttacherImpl::getDbTokens()
{
    TokenList dbTokens;
    foreach (SqliteQueryPtr query, queries)
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
    foreach (Db* db, DBLIST->getValidDbList())
        nameToDbMap[db->getName()] = db;
}

QHash<QString, TokenList> DbAttacherImpl::groupDbTokens(const TokenList& dbTokens)
{
    // Filter out tokens of unknown databases and group results by name
    QHash<QString,TokenList> groupedDbTokens;
    QString strippedName;
    for (TokenPtr token : dbTokens)
    {
        strippedName = stripObjName(token->value, dialect);
        if (!nameToDbMap.contains(strippedName, Qt::CaseInsensitive))
            continue;

        groupedDbTokens[strippedName] += token;
    }
    return groupedDbTokens;
}

bool DbAttacherImpl::attachAllDbs(const QHash<QString, TokenList>& groupedDbTokens)
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
            notifyError(QObject::tr("Could not attach database %1: %2").arg(dbName).arg(db->getErrorText()));
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
    for (TokenPtr srcToken : dbTokens)
    {
        strippedName = stripObjName(srcToken->value, dialect);
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
        foreach (SqliteQueryPtr query, queries)
        {
            idx = query->tokens.indexOf(it.key());
            if (idx < 0)
                continue; // token not in this query, most likely in other query

            query->tokens.replace(idx, it.value());
        }
    }
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
    foreach (SqliteQueryPtr query, queries)
        queryStrings << query->detokenize();

    return queryStrings.join(";");
}

DbAttacher* DbAttacherDefaultFactory::create(Db* db)
{
    return new DbAttacherImpl(db);
}
