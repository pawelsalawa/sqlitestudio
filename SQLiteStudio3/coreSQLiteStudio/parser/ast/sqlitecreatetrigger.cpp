#include "sqlitecreatetrigger.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "sqlitedelete.h"
#include "sqliteinsert.h"
#include "sqliteupdate.h"
#include "sqliteselect.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

SqliteCreateTrigger::SqliteCreateTrigger()
{
    queryType = SqliteQueryType::CreateTrigger;
}

SqliteCreateTrigger::SqliteCreateTrigger(const SqliteCreateTrigger& other) :
    SqliteQuery(other), tempKw(other.tempKw), temporaryKw(other.temporaryKw), ifNotExistsKw(other.ifNotExistsKw), database(other.database),
    trigger(other.trigger), table(other.table), eventTime(other.eventTime), scope(other.scope)
{
    DEEP_COPY_FIELD(Event, event);
    DEEP_COPY_FIELD(SqliteExpr, precondition);

    // Special case of deep collection copy
    SqliteQuery* newQuery = nullptr;
    for (SqliteQuery* query : other.queries)
    {
        switch (query->queryType)
        {
            case SqliteQueryType::Delete:
                newQuery = new SqliteDelete(*dynamic_cast<SqliteDelete*>(query));
                break;
            case SqliteQueryType::Insert:
                newQuery = new SqliteInsert(*dynamic_cast<SqliteInsert*>(query));
                break;
            case SqliteQueryType::Update:
                newQuery = new SqliteUpdate(*dynamic_cast<SqliteUpdate*>(query));
                break;
            case SqliteQueryType::Select:
                newQuery = new SqliteSelect(*dynamic_cast<SqliteSelect*>(query));
                break;
            default:
                newQuery = nullptr;
                break;
        }

        if (!newQuery)
            continue;

        newQuery->setParent(this);
        queries << newQuery;
    }
}

SqliteCreateTrigger::SqliteCreateTrigger(int temp, bool ifNotExists, const QString &name1, const QString &name2, const QString &name3, Time time, SqliteCreateTrigger::Event *event, Scope foreachType, SqliteExpr *when, const QList<SqliteQuery *> &queries, int sqliteVersion) :
    SqliteCreateTrigger()
{
    this->ifNotExistsKw = ifNotExists;
    this->scope = foreachType;
    if (temp == 2)
        temporaryKw = true;
    else if (temp == 1)
        tempKw = true;

    if (sqliteVersion == 3)
    {
        if (name2.isNull())
            trigger = name1;
        else
        {
            database = name1;
            trigger = name2;
        }
        table = name3;
    }
    else
    {
        trigger = name1;
        if (name3.isNull())
            table = name2;
        else
        {
            database = name2;
            table = name3;
        }
    }

    this->event = event;
    eventTime = time;
    this->precondition = when;
    this->queries = queries;

    if (event)
        event->setParent(this);

    if (when)
        when->setParent(this);

    for (SqliteQuery* q : queries)
        q->setParent(this);
}

SqliteCreateTrigger::~SqliteCreateTrigger()
{
}

SqliteStatement*SqliteCreateTrigger::clone()
{
    return new SqliteCreateTrigger(*this);
}

QString SqliteCreateTrigger::getTargetTable() const
{
    return table;
}

QString SqliteCreateTrigger::time(SqliteCreateTrigger::Time eventTime)
{
    switch (eventTime)
    {
        case SqliteCreateTrigger::Time::BEFORE:
            return "BEFORE";
        case SqliteCreateTrigger::Time::AFTER:
            return "AFTER";
        case SqliteCreateTrigger::Time::INSTEAD_OF:
            return "INSTEAD OF";
        case SqliteCreateTrigger::Time::null:
            break;
    }
    return QString();
}

SqliteCreateTrigger::Time SqliteCreateTrigger::time(const QString& eventTime)
{
    if (eventTime == "BEFORE")
        return Time::BEFORE;

    if (eventTime == "AFTER")
        return Time::AFTER;

    if (eventTime == "INSTEAD OF")
        return Time::INSTEAD_OF;

    return Time::null;
}

QString SqliteCreateTrigger::scopeToString(SqliteCreateTrigger::Scope scope)
{
    switch (scope)
    {
        case SqliteCreateTrigger::Scope::FOR_EACH_ROW:
            return "FOR EACH ROW";
        case SqliteCreateTrigger::Scope::FOR_EACH_STATEMENT:
            return "FOR EACH STATEMENT";
        case SqliteCreateTrigger::Scope::null:
            break;
    }
    return QString();
}

SqliteCreateTrigger::Scope SqliteCreateTrigger::stringToScope(const QString& scope)
{
    if (scope == "FOR EACH ROW")
        return Scope::FOR_EACH_ROW;

    if (scope == "FOR EACH STATEMENT")
        return Scope::FOR_EACH_STATEMENT;

    return Scope::null;
}

QStringList SqliteCreateTrigger::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteCreateTrigger::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCreateTrigger::getTableTokensInStatement()
{
    return getTokenListFromNamedKey("nm2");
}

TokenList SqliteCreateTrigger::getDatabaseTokensInStatement()
{
    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteCreateTrigger::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj;
    TokenList tableTokens = getTokenListFromNamedKey("nm2");
    if (tableTokens.size() > 0)
        fullObj = getFullObject(FullObject::TABLE, TokenPtr(), tableTokens[0]);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    // Trigger object
    fullObj = getFullObjectFromNmDbnm(FullObject::TRIGGER, "nm", "dbnm");
    if (fullObj.isValid())
        result << fullObj;

    return result;
}

SqliteCreateTrigger::Event::Event()
{
    this->type = Event::null;
}

SqliteCreateTrigger::Event::Event(SqliteCreateTrigger::Event::Type type)
{
    this->type = type;
}

SqliteCreateTrigger::Event::Event(const SqliteCreateTrigger::Event& other) :
    SqliteStatement(other), type(other.type), columnNames(other.columnNames)
{
}

SqliteCreateTrigger::Event::Event(const QList<QString> &columns)
{
    this->type = UPDATE_OF;
    columnNames = columns;
}

SqliteStatement*SqliteCreateTrigger::Event::clone()
{
    return new SqliteCreateTrigger::Event(*this);
}

TokenList SqliteCreateTrigger::Event::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    switch (type)
    {
        case SqliteCreateTrigger::Event::INSERT:
            builder.withKeyword("INSERT");
            break;
        case SqliteCreateTrigger::Event::UPDATE:
            builder.withKeyword("UPDATE");
            break;
        case SqliteCreateTrigger::Event::DELETE:
            builder.withKeyword("DELETE");
            break;
        case SqliteCreateTrigger::Event::UPDATE_OF:
            builder.withKeyword("UPDATE").withSpace().withKeyword("OF").withSpace().withOtherList(columnNames);
            break;
        case SqliteCreateTrigger::Event::null:
            break;
    }

    return builder.build();
}

QString SqliteCreateTrigger::Event::typeToString(SqliteCreateTrigger::Event::Type type)
{
    switch (type)
    {
        case SqliteCreateTrigger::Event::INSERT:
            return "INSERT";
        case SqliteCreateTrigger::Event::UPDATE:
            return "UPDATE";
        case SqliteCreateTrigger::Event::DELETE:
            return "DELETE";
        case SqliteCreateTrigger::Event::UPDATE_OF:
            return "UPDATE OF";
        case SqliteCreateTrigger::Event::null:
            break;
    }
    return QString();
}

SqliteCreateTrigger::Event::Type SqliteCreateTrigger::Event::stringToType(const QString& type)
{
    if (type == "INSERT")
        return INSERT;

    if (type == "UPDATE")
        return UPDATE;

    if (type == "DELETE")
        return DELETE;

    if (type == "UPDATE OF")
        return UPDATE_OF;

    return Event::null;
}

TokenList SqliteCreateTrigger::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("CREATE").withSpace();
    if (tempKw)
        builder.withKeyword("TEMP").withSpace();
    else if (temporaryKw)
        builder.withKeyword("TEMPORARY").withSpace();

    builder.withKeyword("TRIGGER").withSpace();
    if (ifNotExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(trigger).withSpace();
    switch (eventTime)
    {
        case Time::BEFORE:
            builder.withKeyword("BEFORE").withSpace();
            break;
        case Time::AFTER:
            builder.withKeyword("AFTER").withSpace();
            break;
        case Time::INSTEAD_OF:
            builder.withKeyword("INSTEAD").withSpace().withKeyword("OF").withSpace();
            break;
        case Time::null:
            break;
    }

    builder.withStatement(event).withSpace().withKeyword("ON").withSpace();
    builder.withOther(table).withSpace();

    switch (scope)
    {
        case SqliteCreateTrigger::Scope::FOR_EACH_ROW:
            builder.withKeyword("FOR").withSpace().withKeyword("EACH").withSpace().withKeyword("ROW").withSpace();
            break;
        case SqliteCreateTrigger::Scope::FOR_EACH_STATEMENT:
            builder.withKeyword("FOR").withSpace().withKeyword("EACH").withSpace().withKeyword("STATEMENT").withSpace();
            break;
        case SqliteCreateTrigger::Scope::null:
            break;
    }

    if (precondition)
        builder.withKeyword("WHEN").withStatement(precondition).withSpace();

    builder.withKeyword("BEGIN").withSpace().withStatementList(queries, ";").withOperator(";").withSpace().withKeyword("END");

    builder.withOperator(";");

    return builder.build();
}

QString SqliteCreateTrigger::getTargetDatabase() const
{
    return database;
}

void SqliteCreateTrigger::setTargetDatabase(const QString& database)
{
    this->database = database;
}

QString SqliteCreateTrigger::getObjectName() const
{
    return trigger;
}

void SqliteCreateTrigger::setObjectName(const QString& name)
{
    trigger = name;
}
