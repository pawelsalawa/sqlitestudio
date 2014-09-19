#include "formatforeignkey.h"

FormatForeignKey::FormatForeignKey(SqliteForeignKey* fk) :
    fk(fk)
{
}

void FormatForeignKey::formatInternal()
{
    withKeyword("REFERENCES").withId(fk->foreignTable);

    if (fk->indexedColumns.size() > 0)
        withParDefLeft().withStatementList(fk->indexedColumns).withParDefRight();

    if (fk->conditions.size() > 0)
        withStatementList(fk->conditions, "");

    if (fk->deferrable != SqliteDeferrable::null)
    {
        if (fk->deferrable == SqliteDeferrable::NOT_DEFERRABLE)
            withKeyword("NOT").withKeyword("DEFERRABLE");
        else if (fk->deferrable == SqliteDeferrable::DEFERRABLE)
            withKeyword("DEFERRABLE");

        if (fk->initially != SqliteInitially::null)
            withKeyword("INITIALLY").withKeyword(sqliteInitially(fk->initially));
    }
}


FormatForeignKeyCondition::FormatForeignKeyCondition(SqliteForeignKey::Condition* cond) :
    cond(cond)
{
}

void FormatForeignKeyCondition::formatInternal()
{
    switch (cond->action)
    {
        case SqliteForeignKey::Condition::UPDATE:
            withKeyword("ON").withKeyword("UPDATE");
            formatReaction();
            break;
        case SqliteForeignKey::Condition::INSERT:
            withKeyword("ON").withKeyword("INSERT");
            formatReaction();
            break;
        case SqliteForeignKey::Condition::DELETE:
            withKeyword("ON").withKeyword("DELETE");
            formatReaction();
            break;
        case SqliteForeignKey::Condition::MATCH:
            withKeyword("MATCH").withId(cond->name);
            break;
    }
}

void FormatForeignKeyCondition::formatReaction()
{
    switch (cond->reaction)
    {
        case SqliteForeignKey::Condition::SET_NULL:
            withKeyword("SET").withKeyword("NULL");
            break;
        case SqliteForeignKey::Condition::SET_DEFAULT:
            withKeyword("SET").withKeyword("DEFAULT");
            break;
        case SqliteForeignKey::Condition::CASCADE:
            withKeyword("CASCADE");
            break;
        case SqliteForeignKey::Condition::RESTRICT:
            withKeyword("RESTRICT");
            break;
        case SqliteForeignKey::Condition::NO_ACTION:
            withKeyword("NO").withKeyword("ACTION");
            break;
    }
}
