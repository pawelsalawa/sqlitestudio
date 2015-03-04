#include "formatcreatetrigger.h"
#include "parser/ast/sqliteexpr.h"

FormatCreateTrigger::FormatCreateTrigger(SqliteCreateTrigger* createTrig) :
    createTrig(createTrig)
{
}

void FormatCreateTrigger::formatInternal()
{
    handleExplainQuery(createTrig);
    withKeyword("CREATE");
    if (createTrig->tempKw)
        withKeyword("TEMP");
    else if (createTrig->temporaryKw)
        withKeyword("TEMPORARY");

    withKeyword("TRIGGER");
    if (createTrig->ifNotExistsKw)
        withKeyword("IF").withKeyword("NOT").withKeyword("EXISTS");

    if (dialect == Dialect::Sqlite3 && !createTrig->database.isNull())
        withId(createTrig->database).withIdDot();

    withId(createTrig->trigger);
    switch (createTrig->eventTime)
    {
        case SqliteCreateTrigger::Time::BEFORE:
            withKeyword("BEFORE");
            break;
        case SqliteCreateTrigger::Time::AFTER:
            withKeyword("AFTER");
            break;
        case SqliteCreateTrigger::Time::INSTEAD_OF:
            withKeyword("INSTEAD").withKeyword("OF");
            break;
        case SqliteCreateTrigger::Time::null:
            break;
    }

    withStatement(createTrig->event).withKeyword("ON");
    if (dialect == Dialect::Sqlite2 && !createTrig->database.isNull())
        withId(createTrig->database).withIdDot();

    withId(createTrig->table);

    switch (createTrig->scope)
    {
        case SqliteCreateTrigger::Scope::FOR_EACH_ROW:
            withKeyword("FOR").withKeyword("EACH").withKeyword("ROW");
            break;
        case SqliteCreateTrigger::Scope::FOR_EACH_STATEMENT:
            withKeyword("FOR").withKeyword("EACH").withKeyword("STATEMENT");
            break;
        case SqliteCreateTrigger::Scope::null:
            break;
    }

    if (createTrig->precondition)
        withKeyword("WHEN").withStatement(createTrig->precondition);

    withNewLine().withKeyword("BEGIN").withNewLine().withIncrIndent().withStatementList(createTrig->queries, QString(), ListSeparator::SEMICOLON).withSemicolon();
    withDecrIndent().withKeyword("END").withSemicolon();
}


FormatCreateTriggerEvent::FormatCreateTriggerEvent(SqliteCreateTrigger::Event* ev) :
    ev(ev)
{
}

void FormatCreateTriggerEvent::formatInternal()
{
    switch (ev->type)
    {
        case SqliteCreateTrigger::Event::INSERT:
            withKeyword("INSERT");
            break;
        case SqliteCreateTrigger::Event::UPDATE:
            withKeyword("UPDATE");
            break;
        case SqliteCreateTrigger::Event::DELETE:
            withKeyword("DELETE");
            break;
        case SqliteCreateTrigger::Event::UPDATE_OF:
            withKeyword("UPDATE").withKeyword("OF").withIdList(ev->columnNames);
            break;
        case SqliteCreateTrigger::Event::null:
            break;
    }
}
