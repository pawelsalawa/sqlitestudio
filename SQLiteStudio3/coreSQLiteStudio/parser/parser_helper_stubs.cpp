#include "parser_helper_stubs.h"
#include "ast/sqlitecreatetable.h"

ParserStubAlias::ParserStubAlias(const QString &name, bool asKw)
{
    this->name = name;
    this->asKw = asKw;
}

ParserIndexedBy::ParserIndexedBy(const QString &name)
{
    indexedBy = name;
}

ParserIndexedBy::ParserIndexedBy(bool notIndexed)
{
    this->notIndexedKw = notIndexed;
}


ParserStubInsertOrReplace::ParserStubInsertOrReplace(bool replace)
{
    this->replace = replace;
    this->orConflict = SqliteConflictAlgo::null;
}

ParserStubInsertOrReplace::ParserStubInsertOrReplace(bool replace, SqliteConflictAlgo orConflict)
{
    this->replace = replace;
    this->orConflict = orConflict;
}


ParserStubExplain::ParserStubExplain(bool explain, bool queryPlan)
{
    this->explain = explain;
    this->queryPlan = queryPlan;
}


ParserDeferSubClause::ParserDeferSubClause(SqliteDeferrable deferrable, SqliteInitially initially)
{
    this->initially = initially;
    this->deferrable = deferrable;
}


ParserTermOrLiteral::ParserTermOrLiteral(const QString& name)
{
    value = name;
    nameMode = true;
}

ParserTermOrLiteral::ParserTermOrLiteral(const QVariant& literal)
{
    value = literal;
}

QString ParserTermOrLiteral::toName() const
{
    return value.toString();
}

QVariant ParserTermOrLiteral::toLiteral() const
{
    return value;
}

bool ParserTermOrLiteral::isName() const
{
    return !value.isNull() && value.userType() == QMetaType::QString;
}

bool ParserTermOrLiteral::isLiteral() const
{
    return !nameMode;
}

ParserStubCreateTableOption* parserStubFindCreateTableOption(const QList<ParserStubCreateTableOption*>& options, ParserStubCreateTableOption::Type type)
{
    return findFirst<ParserStubCreateTableOption>(options, [type](auto opt) -> bool {return opt->type == type;});
}

ParserStubCreateTableOption::ParserStubCreateTableOption(Type type) :
    type(type)
{
}
