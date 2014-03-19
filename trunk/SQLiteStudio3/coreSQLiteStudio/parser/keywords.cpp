#include "keywords.h"
#include "sqlite3_parse.h"
#include "sqlite2_parse.h"
#include <QDebug>
#include <QList>

QHash<QString,int> keywords2;
QHash<QString,int> keywords3;
QSet<QString> rowIdKeywords;
QStringList joinKeywords;
QStringList fkMatchKeywords;
QStringList conflictAlgoKeywords;

int getKeywordId2(const QString& str)
{
    QString upStr = str.toUpper();
    if (keywords2.contains(upStr))
        return keywords2[upStr];
    else
        return TK2_ID;
}

int getKeywordId3(const QString& str)
{
    QString upStr = str.toUpper();
    if (keywords3.contains(upStr))
        return keywords3[upStr];
    else
        return TK3_ID;
}

bool isRowIdKeyword(const QString& str)
{
    return rowIdKeywords.contains(str.toUpper());
}

const QHash<QString,int>& getKeywords2()
{
    return keywords2;
}

const QHash<QString,int>& getKeywords3()
{
    return keywords3;
}

void initKeywords()
{
    // SQLite 3
    keywords3["REINDEX"] = TK3_REINDEX;
    keywords3["INDEXED"] = TK3_INDEXED;
    keywords3["INDEX"] = TK3_INDEX;
    keywords3["DESC"] = TK3_DESC;
    keywords3["ESCAPE"] = TK3_ESCAPE;
    keywords3["EACH"] = TK3_EACH;
    keywords3["CHECK"] = TK3_CHECK;
    keywords3["KEY"] = TK3_KEY;
    keywords3["BEFORE"] = TK3_BEFORE;
    keywords3["FOREIGN"] = TK3_FOREIGN;
    keywords3["FOR"] = TK3_FOR;
    keywords3["IGNORE"] = TK3_IGNORE;
    keywords3["REGEXP"] = TK3_LIKE_KW;
    keywords3["EXPLAIN"] = TK3_EXPLAIN;
    keywords3["INSTEAD"] = TK3_INSTEAD;
    keywords3["ADD"] = TK3_ADD;
    keywords3["DATABASE"] = TK3_DATABASE;
    keywords3["AS"] = TK3_AS;
    keywords3["SELECT"] = TK3_SELECT;
    keywords3["TABLE"] = TK3_TABLE;
    keywords3["LEFT"] = TK3_JOIN_KW;
    keywords3["THEN"] = TK3_THEN;
    keywords3["END"] = TK3_END;
    keywords3["DEFERRABLE"] = TK3_DEFERRABLE;
    keywords3["ELSE"] = TK3_ELSE;
    keywords3["EXCEPT"] = TK3_EXCEPT;
    keywords3["TRANSACTION"] = TK3_TRANSACTION;
    keywords3["ACTION"] = TK3_ACTION;
    keywords3["ON"] = TK3_ON;
    keywords3["NATURAL"] = TK3_JOIN_KW;
    keywords3["ALTER"] = TK3_ALTER;
    keywords3["RAISE"] = TK3_RAISE;
    keywords3["EXCLUSIVE"] = TK3_EXCLUSIVE;
    keywords3["EXISTS"] = TK3_EXISTS;
    keywords3["SAVEPOINT"] = TK3_SAVEPOINT;
    keywords3["INTERSECT"] = TK3_INTERSECT;
    keywords3["TRIGGER"] = TK3_TRIGGER;
    keywords3["REFERENCES"] = TK3_REFERENCES;
    keywords3["CONSTRAINT"] = TK3_CONSTRAINT;
    keywords3["INTO"] = TK3_INTO;
    keywords3["OFFSET"] = TK3_OFFSET;
    keywords3["OF"] = TK3_OF;
    keywords3["SET"] = TK3_SET;
    keywords3["TEMP"] = TK3_TEMP;
    keywords3["TEMPORARY"] = TK3_TEMP;
    keywords3["OR"] = TK3_OR;
    keywords3["UNIQUE"] = TK3_UNIQUE;
    keywords3["QUERY"] = TK3_QUERY;
    keywords3["ATTACH"] = TK3_ATTACH;
    keywords3["HAVING"] = TK3_HAVING;
    keywords3["GROUP"] = TK3_GROUP;
    keywords3["UPDATE"] = TK3_UPDATE;
    keywords3["BEGIN"] = TK3_BEGIN;
    keywords3["INNER"] = TK3_JOIN_KW;
    keywords3["RELEASE"] = TK3_RELEASE;
    keywords3["BETWEEN"] = TK3_BETWEEN;
    keywords3["NOTNULL"] = TK3_NOTNULL;
    keywords3["NOT"] = TK3_NOT;
    keywords3["NO"] = TK3_NO;
    keywords3["NULL"] = TK3_NULL;
    keywords3["LIKE"] = TK3_LIKE_KW;
    keywords3["CASCADE"] = TK3_CASCADE;
    keywords3["ASC"] = TK3_ASC;
    keywords3["DELETE"] = TK3_DELETE;
    keywords3["CASE"] = TK3_CASE;
    keywords3["COLLATE"] = TK3_COLLATE;
    keywords3["CREATE"] = TK3_CREATE;
    keywords3["CURRENT_DATE"] = TK3_CTIME_KW;
    keywords3["DETACH"] = TK3_DETACH;
    keywords3["IMMEDIATE"] = TK3_IMMEDIATE;
    keywords3["JOIN"] = TK3_JOIN;
    keywords3["INSERT"] = TK3_INSERT;
    keywords3["MATCH"] = TK3_MATCH;
    keywords3["PLAN"] = TK3_PLAN;
    keywords3["ANALYZE"] = TK3_ANALYZE;
    keywords3["PRAGMA"] = TK3_PRAGMA;
    keywords3["ABORT"] = TK3_ABORT;
    keywords3["VALUES"] = TK3_VALUES;
    keywords3["VIRTUAL"] = TK3_VIRTUAL;
    keywords3["LIMIT"] = TK3_LIMIT;
    keywords3["WHEN"] = TK3_WHEN;
    keywords3["WHERE"] = TK3_WHERE;
    keywords3["RENAME"] = TK3_RENAME;
    keywords3["AFTER"] = TK3_AFTER;
    keywords3["REPLACE"] = TK3_REPLACE;
    keywords3["AND"] = TK3_AND;
    keywords3["DEFAULT"] = TK3_DEFAULT;
    keywords3["AUTOINCREMENT"] = TK3_AUTOINCR;
    keywords3["TO"] = TK3_TO;
    keywords3["IN"] = TK3_IN;
    keywords3["CAST"] = TK3_CAST;
    keywords3["COLUMN"] = TK3_COLUMNKW;
    keywords3["COMMIT"] = TK3_COMMIT;
    keywords3["CONFLICT"] = TK3_CONFLICT;
    keywords3["CROSS"] = TK3_JOIN_KW;
    keywords3["CURRENT_TIMESTAMP"] = TK3_CTIME_KW;
    keywords3["CURRENT_TIME"] = TK3_CTIME_KW;
    keywords3["PRIMARY"] = TK3_PRIMARY;
    keywords3["DEFERRED"] = TK3_DEFERRED;
    keywords3["DISTINCT"] = TK3_DISTINCT;
    keywords3["IS"] = TK3_IS;
    keywords3["DROP"] = TK3_DROP;
    keywords3["FAIL"] = TK3_FAIL;
    keywords3["FROM"] = TK3_FROM;
    keywords3["FULL"] = TK3_JOIN_KW;
    keywords3["GLOB"] = TK3_LIKE_KW;
    keywords3["BY"] = TK3_BY;
    keywords3["IF"] = TK3_IF;
    keywords3["ISNULL"] = TK3_ISNULL;
    keywords3["ORDER"] = TK3_ORDER;
    keywords3["RESTRICT"] = TK3_RESTRICT;
    keywords3["OUTER"] = TK3_JOIN_KW;
    keywords3["RIGHT"] = TK3_JOIN_KW;
    keywords3["ROLLBACK"] = TK3_ROLLBACK;
    keywords3["ROW"] = TK3_ROW;
    keywords3["UNION"] = TK3_UNION;
    keywords3["USING"] = TK3_USING;
    keywords3["VACUUM"] = TK3_VACUUM;
    keywords3["VIEW"] = TK3_VIEW;
    keywords3["INITIALLY"] = TK3_INITIALLY;
    keywords3["WITHOUT"] = TK3_WITHOUT;
    keywords3["ALL"] = TK3_ALL;
    keywords3["WITH"] = TK3_WITH;
    keywords3["RECURSIVE"] = TK3_RECURSIVE;

    // SQLite 2
    keywords2["ABORT"] = TK2_ABORT;
    keywords2["AFTER"] = TK2_AFTER;
    keywords2["ALL"] = TK2_ALL;
    keywords2["AND"] = TK2_AND;
    keywords2["AS"] = TK2_AS;
    keywords2["ASC"] = TK2_ASC;
    keywords2["ATTACH"] = TK2_ATTACH;
    keywords2["BEFORE"] = TK2_BEFORE;
    keywords2["BEGIN"] = TK2_BEGIN;
    keywords2["BETWEEN"] = TK2_BETWEEN;
    keywords2["BY"] = TK2_BY;
    keywords2["CASCADE"] = TK2_CASCADE;
    keywords2["CASE"] = TK2_CASE;
    keywords2["CHECK"] = TK2_CHECK;
    keywords2["CLUSTER"] = TK2_CLUSTER;
    keywords2["COLLATE"] = TK2_COLLATE;
    keywords2["COMMIT"] = TK2_COMMIT;
    keywords2["CONFLICT"] = TK2_CONFLICT;
    keywords2["CONSTRAINT"] = TK2_CONSTRAINT;
    keywords2["COPY"] = TK2_COPY;
    keywords2["CREATE"] = TK2_CREATE;
    keywords2["CROSS"] = TK2_JOIN_KW;
    keywords2["DATABASE"] = TK2_DATABASE;
    keywords2["DEFAULT"] = TK2_DEFAULT;
    keywords2["DEFERRED"] = TK2_DEFERRED;
    keywords2["DEFERRABLE"] = TK2_DEFERRABLE;
    keywords2["DELETE"] = TK2_DELETE;
    keywords2["DELIMITERS"] = TK2_DELIMITERS;
    keywords2["DESC"] = TK2_DESC;
    keywords2["DETACH"] = TK2_DETACH;
    keywords2["DISTINCT"] = TK2_DISTINCT;
    keywords2["DROP"] = TK2_DROP;
    keywords2["END"] = TK2_END;
    keywords2["EACH"] = TK2_EACH;
    keywords2["ELSE"] = TK2_ELSE;
    keywords2["EXCEPT"] = TK2_EXCEPT;
    keywords2["EXPLAIN"] = TK2_EXPLAIN;
    keywords2["FAIL"] = TK2_FAIL;
    keywords2["FOR"] = TK2_FOR;
    keywords2["FOREIGN"] = TK2_FOREIGN;
    keywords2["FROM"] = TK2_FROM;
    keywords2["FULL"] = TK2_JOIN_KW;
    keywords2["GLOB"] = TK2_GLOB;
    keywords2["GROUP"] = TK2_GROUP;
    keywords2["HAVING"] = TK2_HAVING;
    keywords2["IGNORE"] = TK2_IGNORE;
    keywords2["IMMEDIATE"] = TK2_IMMEDIATE;
    keywords2["IN"] = TK2_IN;
    keywords2["INDEX"] = TK2_INDEX;
    keywords2["INITIALLY"] = TK2_INITIALLY;
    keywords2["INNER"] = TK2_JOIN_KW;
    keywords2["INSERT"] = TK2_INSERT;
    keywords2["INSTEAD"] = TK2_INSTEAD;
    keywords2["INTERSECT"] = TK2_INTERSECT;
    keywords2["INTO"] = TK2_INTO;
    keywords2["IS"] = TK2_IS;
    keywords2["ISNULL"] = TK2_ISNULL;
    keywords2["JOIN"] = TK2_JOIN;
    keywords2["KEY"] = TK2_KEY;
    keywords2["LEFT"] = TK2_JOIN_KW;
    keywords2["LIKE"] = TK2_LIKE;
    keywords2["LIMIT"] = TK2_LIMIT;
    keywords2["MATCH"] = TK2_MATCH;
    keywords2["NATURAL"] = TK2_JOIN_KW;
    keywords2["NOT"] = TK2_NOT;
    keywords2["NOTNULL"] = TK2_NOTNULL;
    keywords2["NULL"] = TK2_NULL;
    keywords2["OF"] = TK2_OF;
    keywords2["OFFSET"] = TK2_OFFSET;
    keywords2["ON"] = TK2_ON;
    keywords2["OR"] = TK2_OR;
    keywords2["ORDER"] = TK2_ORDER;
    keywords2["OUTER"] = TK2_JOIN_KW;
    keywords2["PRAGMA"] = TK2_PRAGMA;
    keywords2["PRIMARY"] = TK2_PRIMARY;
    keywords2["RAISE"] = TK2_RAISE;
    keywords2["REFERENCES"] = TK2_REFERENCES;
    keywords2["REPLACE"] = TK2_REPLACE;
    keywords2["RESTRICT"] = TK2_RESTRICT;
    keywords2["RIGHT"] = TK2_JOIN_KW;
    keywords2["ROLLBACK"] = TK2_ROLLBACK;
    keywords2["ROW"] = TK2_ROW;
    keywords2["SELECT"] = TK2_SELECT;
    keywords2["SET"] = TK2_SET;
    keywords2["STATEMENT"] = TK2_STATEMENT;
    keywords2["TABLE"] = TK2_TABLE;
    keywords2["TEMP"] = TK2_TEMP;
    keywords2["TEMPORARY"] = TK2_TEMP;
    keywords2["THEN"] = TK2_THEN;
    keywords2["TRANSACTION"] = TK2_TRANSACTION;
    keywords2["TRIGGER"] = TK2_TRIGGER;
    keywords2["UNION"] = TK2_UNION;
    keywords2["UNIQUE"] = TK2_UNIQUE;
    keywords2["UPDATE"] = TK2_UPDATE;
    keywords2["USING"] = TK2_USING;
    keywords2["VACUUM"] = TK2_VACUUM;
    keywords2["VALUES"] = TK2_VALUES;
    keywords2["VIEW"] = TK2_VIEW;
    keywords2["WHEN"] = TK2_WHEN;
    keywords2["WHERE"] = TK2_WHERE;

    rowIdKeywords << "_ROWID_"
                  << "ROWID"
                  << "OID";


    joinKeywords << "NATURAL" << "LEFT" << "RIGHT" << "OUTER" << "INNER" << "CROSS";
    fkMatchKeywords << "SIMPLE" << "FULL" << "PARTIAL";
    conflictAlgoKeywords << "ROLLBACK" << "ABORT" << "FAIL" << "IGNORE" << "REPLACE";
}


bool isJoinKeyword(const QString &str)
{
    return joinKeywords.contains(str, Qt::CaseInsensitive);
}

QStringList getJoinKeywords()
{
    return joinKeywords;
}

QStringList getFkMatchKeywords()
{
    return fkMatchKeywords;
}

bool isFkMatchKeyword(const QString &str)
{
    return fkMatchKeywords.contains(str);
}


bool isKeyword(const QString& str, Dialect dialect)
{
    switch (dialect)
    {
        case Dialect::Sqlite3:
            return keywords3.contains(str.toUpper());
        case Dialect::Sqlite2:
            return keywords2.contains(str.toUpper());
    }
    return false;
}

QStringList getConflictAlgorithms()
{
    return conflictAlgoKeywords;
}
