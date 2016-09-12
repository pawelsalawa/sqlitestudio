#include "formatcreatetrigger.h"
#include "parser/ast/sqliteexpr.h"

FormatCreateTrigger::FormatCreateTrigger(SqliteCreateTrigger* createTrig) :
    createTrig(createTrig)
{
}

void FormatCreateTrigger::formatInternal()
{
    handleExplainQuery(createTrig);

    QStringList keywords;

    keywords << "CREATE";
    if (createTrig->tempKw)
        keywords << "TEMP";
    else if (createTrig->temporaryKw)
        keywords << "TEMPORARY";

    keywords << "TRIGGER";
    if (createTrig->ifNotExistsKw)
        keywords << "IF" << "NOT" << "EXISTS";

    QString kwLineUp = keywords.join(" ");
    markKeywordLineUp(kwLineUp, TRIGGER_MARK);

    for (const QString& kw : keywords)
        withKeyword(kw);

    if (dialect == Dialect::Sqlite3 && !createTrig->database.isNull())
        withId(createTrig->database).withIdDot();

    withId(createTrig->trigger).withNewLine();

    FormatStatementEnricher eventStmtEnricher = nullptr;
    switch (createTrig->eventTime)
    {
        case SqliteCreateTrigger::Time::BEFORE:
            withLinedUpKeyword("BEFORE", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Time::AFTER:
            withLinedUpKeyword("AFTER", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Time::INSTEAD_OF:
            withLinedUpKeyword("INSTEAD OF", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Time::null:
            eventStmtEnricher = [kwLineUp](FormatStatement* stmt)
            {
                dynamic_cast<FormatCreateTriggerEvent*>(stmt)->setLineUpKeyword(kwLineUp);
            };
            break;
    }

    withStatement(createTrig->event, QString(), eventStmtEnricher).withNewLine();
    withLinedUpKeyword("ON", TRIGGER_MARK);
    if (dialect == Dialect::Sqlite2 && !createTrig->database.isNull())
        withId(createTrig->database).withIdDot();

    withId(createTrig->table).withNewLine();

    switch (createTrig->scope)
    {
        case SqliteCreateTrigger::Scope::FOR_EACH_ROW:
            withLinedUpKeyword("FOR EACH", TRIGGER_MARK).withKeyword("ROW").withNewLine();
            break;
        case SqliteCreateTrigger::Scope::FOR_EACH_STATEMENT:
            withLinedUpKeyword("FOR EACH", TRIGGER_MARK).withKeyword("STATEMENT").withNewLine();
            break;
        case SqliteCreateTrigger::Scope::null:
            break;
    }

    if (createTrig->precondition)
        withLinedUpKeyword("WHEN", TRIGGER_MARK).withStatement(createTrig->precondition);

    withNewLine().withKeyword("BEGIN").withNewLine().withIncrIndent().withStatementList(createTrig->queries, QString(), ListSeparator::SEMICOLON).withSemicolon();
    withDecrIndent().withKeyword("END").withSemicolon();
}


FormatCreateTriggerEvent::FormatCreateTriggerEvent(SqliteCreateTrigger::Event* ev) :
    ev(ev)
{
}

void FormatCreateTriggerEvent::setLineUpKeyword(const QString& lineUpKw)
{
    this->lineUpKw = lineUpKw;
}

void FormatCreateTriggerEvent::formatInternal()
{
    if (!lineUpKw.isNull())
        markKeywordLineUp(lineUpKw, TRIGGER_MARK);

    switch (ev->type)
    {
        case SqliteCreateTrigger::Event::INSERT:
            withLinedUpKeyword("INSERT", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Event::UPDATE:
            withLinedUpKeyword("UPDATE", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Event::DELETE:
            withLinedUpKeyword("DELETE", TRIGGER_MARK);
            break;
        case SqliteCreateTrigger::Event::UPDATE_OF:
            withLinedUpKeyword("UPDATE OF", TRIGGER_MARK).withIdList(ev->columnNames, "updateOfCols");
            break;
        case SqliteCreateTrigger::Event::null:
            break;
    }
}
