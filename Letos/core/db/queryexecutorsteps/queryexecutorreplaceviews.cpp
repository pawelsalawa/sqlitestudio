#include "queryexecutorreplaceviews.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqliteselect.h"
#include "schemaresolver.h"
#include "common/utils_sql.h"
#include <QDebug>

static void qualifyViewSources(SqliteSelect* select, const QString& database)
{
    auto qualifySource = [&](SqliteStatement* source, QString& sourceDatabase, const QString& table)
    {
        if (table.isNull() || !sourceDatabase.isNull())
            return;

        for (SqliteStatement* stmt = source->parentStatement(); stmt; stmt = stmt->parentStatement())
        {
            auto scopedSelect = dynamic_cast<SqliteSelect*>(stmt);
            if (!scopedSelect || !scopedSelect->with)
                continue;

            for (auto cte : scopedSelect->with->cteList)
                if (stripObjName(cte->table).compare(stripObjName(table), Qt::CaseInsensitive) == 0)
                    return;
        }
        sourceDatabase = database;
    };

    for (auto source : select->getAllTypedStatements<SqliteSelect::Core::SingleSource>())
        qualifySource(source, source->database, source->table);

    for (auto expr : select->getAllTypedStatements<SqliteExpr>())
    {
        if (expr->mode == SqliteExpr::Mode::IN)
            qualifySource(expr, expr->database, expr->table);
    }
}

QueryExecutorReplaceViews::~QueryExecutorReplaceViews()
{
    delete schemaResolver;
    schemaResolver = nullptr;
}

bool QueryExecutorReplaceViews::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    if (select->coreSelects.size() > 1)
        return true;

    if (select->coreSelects.first()->distinctKw)
        return true;

    replaceViews(select.data());
    select->rebuildTokens();
    updateQueries();

    return true;
}

void QueryExecutorReplaceViews::init()
{
    if (!schemaResolver)
        schemaResolver = new SchemaResolver(db);
}

QStringList QueryExecutorReplaceViews::getViews(const QString& database)
{
    QString dbName = database.isNull() ? "main" : database.toLower();
    if (views.contains(dbName))
        return views[dbName];

    views[dbName] = schemaResolver->getViews(database);
    return views[dbName];
}

SqliteCreateViewPtr QueryExecutorReplaceViews::getView(const QString& database, const QString& viewName)
{
    View view(database, viewName);
    if (viewStatements.contains(view))
        return viewStatements[view];

    SqliteQueryPtr query = schemaResolver->getParsedObject(database, viewName, SchemaResolver::VIEW);
    if (!query)
        return SqliteCreateViewPtr();

    SqliteCreateViewPtr viewPtr = query.dynamicCast<SqliteCreateView>();
    if (!viewPtr)
        return SqliteCreateViewPtr();

    viewStatements[view] = viewPtr;
    return viewPtr;
}

void QueryExecutorReplaceViews::replaceViews(SqliteSelect* select)
{
    SqliteSelect::Core* core = select->coreSelects.first();
    QList<SqliteSelect::Core::SingleSource*> sources = core->getAllTypedStatements<SqliteSelect::Core::SingleSource>();

    typedef QPair<SqliteSelect::Core::SingleSource*, SqliteCreateViewPtr> SourceViewPair;
    SqliteCreateViewPtr view;
    QList<SourceViewPair> sourceViewPairs;
    for (SqliteSelect::Core::SingleSource* src : sources)
    {
        if (src->table.isNull())
            continue;

        // TEMP views can refer to multiple schemas, so retain their native name resolution.
        if (src->database.compare("temp", Qt::CaseInsensitive) == 0)
            continue;

        QStringList viewsInDatabase = getViews(src->database);
        if (!viewsInDatabase.contains(src->table, Qt::CaseInsensitive))
            continue;

        view = getView(src->database, src->table);
        if (!view)
        {
            qWarning() << "Object" << src->database << "." << src->table
                       << "was identified to be a view, but could not get it's parsed representation.";
            continue;
        }

        if (usesAnyView(view->select, viewsInDatabase))
        {
            // Multi-level views (view selecting from view, selecting from view...).
            // Such constructs build up easily to huge, non-optimized queries.
            // For performance reasons, we won't expand such views.
            qDebug() << "Multi-level views. Skipping view expanding feature of query executor. Some columns won't be editable due to that.";
            return;
        }

        sourceViewPairs << SourceViewPair(src, view);
    }

    for (SourceViewPair& pair : sourceViewPairs)
    {
        view = pair.second;

        QString alias = pair.first->alias.isNull() ? view->view : pair.first->alias;

        QString sql = view->equivalentSelectTokens().detokenize();
        Parser parser;
        if (!parser.parse(sql) || parser.getQueries().size() == 0)
            qWarning() << "Couldn't replace view " << view->view << " by equivalent select " << sql;
        else
        {
            pair.first->select = new SqliteSelect(*dynamic_cast<SqliteSelect *>(parser.getQueries().first().get()));
            QString database = pair.first->database.isNull() ? "main" : pair.first->database;
            qualifyViewSources(pair.first->select, database);
            pair.first->select->setParent(pair.first);
            pair.first->alias = alias;
            pair.first->database = QString();
            pair.first->table = QString();

            // replaceViews(pair.first->select); // No recursion, as we avoid multi-level expanding.
        }
    }

    context->viewsExpanded = true;
}

bool QueryExecutorReplaceViews::usesAnyView(SqliteSelect* select, const QStringList& viewsInDatabase)
{
    for (SqliteSelect::Core*& core : select->coreSelects)
    {
        QList<SqliteSelect::Core::SingleSource*> sources = core->getAllTypedStatements<SqliteSelect::Core::SingleSource>();
        for (SqliteSelect::Core::SingleSource* src : sources)
        {
            if (src->table.isNull())
                continue;

            if (viewsInDatabase.contains(src->table, Qt::CaseInsensitive))
                return true;
        }
    }
    return false;
}

size_t qHash(const QueryExecutorReplaceViews::View& view)
{
    return qHash(view.database + "." + view.view);
}

QueryExecutorReplaceViews::View::View(const QString& database, const QString& view) :
    database(database), view(view)
{
}

int QueryExecutorReplaceViews::View::operator==(const QueryExecutorReplaceViews::View& other) const
{
    return database == other.database && view == other.view;
}
