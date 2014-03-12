#include "sqliteforeignkey.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include <QDebug>

SqliteForeignKey::Condition::Condition(SqliteForeignKey::Condition::Action action, SqliteForeignKey::Condition::Reaction reaction)
{
    this->action = action;
    this->reaction = reaction;
}

SqliteForeignKey::Condition::Condition(const QString &name)
{
    this->action = SqliteForeignKey::Condition::MATCH;
    this->name = name;
}

SqliteForeignKey::Condition::Condition(const SqliteForeignKey::Condition& other) :
    SqliteStatement(other), action(other.action), name(other.name), reaction(other.reaction)
{
}

QString SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::Reaction reaction)
{
    switch (reaction)
    {
        case SqliteForeignKey::Condition::SET_NULL:
            return "SET NULL";
        case SqliteForeignKey::Condition::SET_DEFAULT:
            return "SET DEFAULT";
        case SqliteForeignKey::Condition::CASCADE:
            return "CASCADE";
        case SqliteForeignKey::Condition::RESTRICT:
            return "RESTRICT";
        case SqliteForeignKey::Condition::NO_ACTION:
            return "NO ACTION";
    }
    return QString::null;
}

SqliteForeignKey::Condition::Reaction SqliteForeignKey::Condition::toEnum(const QString& reaction)
{
    QString upper = reaction.toUpper();
    if (upper == "SET NULL")
        return SET_NULL;

    if (upper == "SET DEFAULT")
        return SET_DEFAULT;

    if (upper == "CASCADE")
        return CASCADE;

    if (upper == "RESTRICT")
        return RESTRICT;

    if (upper == "NO ACTION")
        return NO_ACTION;

    qCritical() << "Unknown Reaction value. Cannot convert to Condition::Reaction. Returning default, the SET_NULL.";
    return SET_NULL;
}

SqliteForeignKey::SqliteForeignKey()
{
}

SqliteForeignKey::SqliteForeignKey(const SqliteForeignKey& other) :
    SqliteStatement(other), foreignTable(other.foreignTable), deferrable(other.deferrable), initially(other.initially)
{
    DEEP_COPY_COLLECTION(SqliteIndexedColumn, indexedColumns);
    DEEP_COPY_COLLECTION(Condition, conditions);
}

SqliteForeignKey::~SqliteForeignKey()
{
}

QStringList SqliteForeignKey::getTablesInStatement()
{
    return getStrListFromValue(foreignTable);
}

TokenList SqliteForeignKey::getTableTokensInStatement()
{
    return getTokenListFromNamedKey("nm");
}

QList<SqliteStatement::FullObject> SqliteForeignKey::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj;
    TokenList tokens = getTableTokensInStatement();
    if (tokens.size() > 0)
        fullObj = getFullObject(FullObject::TABLE, dbTokenForFullObjects, tokens[0]);

    if (fullObj.isValid())
        result << fullObj;

    return result;
}

TokenList SqliteForeignKey::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("REFERENCES").withSpace().withOther(foreignTable, dialect);

    if (indexedColumns.size() > 0)
        builder.withSpace().withParLeft().withStatementList(indexedColumns).withParRight();

    if (conditions.size() > 0)
        builder.withSpace().withStatementList(conditions, "");

    if (deferrable != SqliteDeferrable::null)
    {
        if (deferrable == SqliteDeferrable::NOT_DEFERRABLE)
            builder.withSpace().withKeyword("NOT").withSpace().withKeyword("DEFERRABLE");
        else if (deferrable == SqliteDeferrable::DEFERRABLE)
            builder.withSpace().withKeyword("DEFERRABLE");

        if (initially != SqliteInitially::null)
            builder.withSpace().withKeyword("INITIALLY").withSpace().withKeyword(sqliteInitially(initially));
    }

    return builder.build();
}


TokenList SqliteForeignKey::Condition::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    switch (action)
    {
        case SqliteForeignKey::Condition::UPDATE:
            builder.withKeyword("ON").withSpace().withKeyword("UPDATE").withSpace();
            applyReactionToBuilder(builder);
            break;
        case SqliteForeignKey::Condition::INSERT:
            builder.withKeyword("ON").withSpace().withKeyword("INSERT").withSpace();
            applyReactionToBuilder(builder);
            break;
        case SqliteForeignKey::Condition::DELETE:
            builder.withKeyword("ON").withSpace().withKeyword("DELETE").withSpace();
            applyReactionToBuilder(builder);
            break;
        case SqliteForeignKey::Condition::MATCH:
            builder.withKeyword("MATCH").withSpace().withOther(name);
            break;

    }

    return builder.build();
}

void SqliteForeignKey::Condition::applyReactionToBuilder(StatementTokenBuilder& builder)
{
    switch (reaction)
    {
        case SqliteForeignKey::Condition::SET_NULL:
            builder.withKeyword("SET").withSpace().withKeyword("NULL");
            break;
        case SqliteForeignKey::Condition::SET_DEFAULT:
            builder.withKeyword("SET").withSpace().withKeyword("DEFAULT");
            break;
        case SqliteForeignKey::Condition::CASCADE:
            builder.withKeyword("CASCADE");
            break;
        case SqliteForeignKey::Condition::RESTRICT:
            builder.withKeyword("RESTRICT");
            break;
        case SqliteForeignKey::Condition::NO_ACTION:
            builder.withKeyword("NO").withSpace().withKeyword("ACTION");
            break;
    }
}
