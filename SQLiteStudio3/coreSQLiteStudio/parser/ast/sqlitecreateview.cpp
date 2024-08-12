#include "sqlitecreateview.h"
#include "sqliteselect.h"
#include "sqlitequerytype.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqliteindexedcolumn.h"
#include <QDebug>

SqliteCreateView::SqliteCreateView()
{
    queryType = SqliteQueryType::CreateView;
}

SqliteCreateView::SqliteCreateView(const SqliteCreateView& other) :
    SqliteQuery(other), tempKw(other.tempKw), temporaryKw(other.temporaryKw), ifNotExists(other.ifNotExists),
    database(other.database), view(other.view)
{
    DEEP_COPY_FIELD(SqliteSelect, select);
    DEEP_COPY_COLLECTION(SqliteIndexedColumn, columns);
}

SqliteCreateView::SqliteCreateView(int temp, bool ifNotExists, const QString &name1, const QString &name2, SqliteSelect *select) :
    SqliteCreateView()
{
    this->ifNotExists = ifNotExists;

    if (name2.isNull())
        view = name1;
    else
    {
        database = name1;
        view = name2;
    }

    if (temp == 2)
        temporaryKw = true;
    else if (temp == 1)
        tempKw = true;

    this->select = select;

    if (select)
        select->setParent(this);
}

SqliteCreateView::SqliteCreateView(int temp, bool ifNotExists, const QString& name1, const QString& name2, SqliteSelect* select, const QList<SqliteIndexedColumn*>& columns)
    : SqliteCreateView(temp, ifNotExists, name1, name2, select)
{
    this->columns = columns;

    for (SqliteIndexedColumn* col : columns)
        col->setParent(this);
}

SqliteCreateView::~SqliteCreateView()
{
}

SqliteStatement*SqliteCreateView::clone()
{
    return new SqliteCreateView(*this);
}

QStringList SqliteCreateView::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCreateView::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteCreateView::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // View object
    FullObject fullObj = getFullObjectFromFullname(FullObject::VIEW);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    return result;
}

TokenList SqliteCreateView::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("CREATE").withSpace();
    if (tempKw)
        builder.withKeyword("TEMP").withSpace();
    else if (temporaryKw)
        builder.withKeyword("TEMPORARY").withSpace();

    builder.withKeyword("VIEW").withSpace();
    if (ifNotExists)
        builder.withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(view).withSpace();

    if (columns.size() > 0)
        builder.withParLeft().withStatementList<SqliteIndexedColumn>(columns).withParRight().withSpace();

    builder.withKeyword("AS").withStatement(select);

    builder.withOperator(";");

    return builder.build();
}

QString SqliteCreateView::getTargetDatabase() const
{
    return database;
}

void SqliteCreateView::setTargetDatabase(const QString& database)
{
    this->database = database;
}

QString SqliteCreateView::getObjectName() const
{
    return view;
}

void SqliteCreateView::setObjectName(const QString& name)
{
    view = name;
}

TokenList SqliteCreateView::equivalentSelectTokens() const
{
    if (columns.size() == 0)
        return select->tokens;

    SqliteSelect::Core *core = select->coreSelects.first();
    bool hasStar = std::any_of(core->resultColumns.cbegin(),
                               core->resultColumns.cend(),
                               [](auto col) { return col->star; });
    bool useCTE = false;
    if (!hasStar && columns.size() != core->resultColumns.size())
    {
        qWarning() << "View with a column list clause and non-matching count of columns in SELECT. "
                   << "Expect an error if result column count does not match column list length.";
        useCTE = true;
    }
    if (hasStar)
    {
        qWarning() << "View with a column list clause and SELECT *. "
                   << "Expect an error if result column count does not match column list length.";
        useCTE = true;
    }
    if (useCTE)
    {
        StatementTokenBuilder builder;
        builder.withKeyword("WITH").withSpace().withOther(view).withParLeft();
        bool first = true;
        for (SqliteIndexedColumn *column : columns)
        {
            if (!first)
                builder.withOperator(",");
            first = false;
            builder.withOther(column->name);
        }
        builder.withParRight().withKeyword("AS").withParLeft().withTokens(select->tokens).withParRight()
            .withKeyword("SELECT").withSpace().withOperator("*").withSpace()
            .withKeyword("FROM").withSpace().withOther(view);
        return builder.build();
    }

    // There are no * in SELECT, and view column list length matches SELECT column list length.
    // Apply view column names to SELECT as aliases.
    SqliteSelect* selectCopy = dynamic_cast<SqliteSelect*>(select->clone());
    QList<SqliteSelect::Core::ResultColumn*> resultColumns = selectCopy->coreSelects.first()->resultColumns;
    int n = 0;
    for (SqliteSelect::Core::ResultColumn* column : resultColumns)
    {
        column->asKw = true;
        column->alias = columns.at(n++)->name;
    }
    selectCopy->rebuildTokens();
    return selectCopy->tokens;
}
