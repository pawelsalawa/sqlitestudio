#include "viewmodifier.h"
#include "utils_sql.h"
#include "parser/parser.h"
#include "schemaresolver.h"
#include "selectresolver.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "unused.h"

ViewModifier::ViewModifier(Db* db, const QString& view) :
    ViewModifier(db, "main", view)
{
}

ViewModifier::ViewModifier(Db* db, const QString& database, const QString& view) :
    db(db), database(database), view(view)
{
    dialect = db->getDialect();
}

void ViewModifier::alterView(const QString& newView)
{
    Parser parser(dialect);
    if (!parser.parse(newView) || parser.getQueries().size() == 0)
    {
        errors << QObject::tr("Could not parse DDL of the view to be created. Details: %1").arg(parser.getErrorString());
        return;
    }

    SqliteQueryPtr query = parser.getQueries().first();
    createView = query.dynamicCast<SqliteCreateView>();

    if (!createView)
    {
        errors << QObject::tr("Parsed query is not CREATE VIEW. It's: %1").arg(sqliteQueryTypeToString(query->queryType));
        return;
    }

    alterView(createView);
}

void ViewModifier::alterView(SqliteCreateViewPtr newView)
{
    createView = newView;

    addMandatorySql(QString("DROP VIEW %1").arg(wrapObjIfNeeded(view, dialect)));
    addMandatorySql(newView->detokenize());

    collectNewColumns();
    handleTriggers();

    // TODO handle other views selecting from this view
}

void ViewModifier::handleTriggers()
{
    SchemaResolver resolver(db);
    QList<SqliteCreateTriggerPtr> triggers = resolver.getParsedTriggersForView(view, true);
    foreach (SqliteCreateTriggerPtr trigger, triggers)
    {
        addOptionalSql(QString("DROP TRIGGER %1").arg(wrapObjIfNeeded(trigger->trigger, dialect)));

        if (!handleNewColumns(trigger))
            continue;

        addOptionalSql(trigger->detokenize());
    }
}

bool ViewModifier::handleNewColumns(SqliteCreateTriggerPtr trigger)
{
    UNUSED(trigger);
    // TODO update all occurances of columns in "UPDATE OF" and statements inside, just like it would be done in TableModifier.
    return true;
}

void ViewModifier::collectNewColumns()
{
    SelectResolver resolver(db, createView->select->detokenize());
    QList<QList<SelectResolver::Column> > multiColumns = resolver.resolve(createView->select);
    if (multiColumns.size() < 1)
    {
        warnings << QObject::tr("SQLiteStudio was unable to resolve columns returned by the new view, "
                                "therefore it won't be able to tell which triggers might fail during the recreation process.");
        return;
    }

    foreach (const SelectResolver::Column& col, multiColumns.first())
        newColumns << col.column;
}

void ViewModifier::addMandatorySql(const QString& sql)
{
    sqls << sql;
    sqlMandatoryFlags << true;
}

void ViewModifier::addOptionalSql(const QString& sql)
{

    sqls << sql;
    sqlMandatoryFlags << false;
}

QStringList ViewModifier::generateSqls() const
{
    return sqls;
}

QList<bool> ViewModifier::getMandatoryFlags() const
{
    return sqlMandatoryFlags;
}

QStringList ViewModifier::getWarnings() const
{
    return warnings;
}

QStringList ViewModifier::getErrors() const
{
    return errors;
}

bool ViewModifier::hasMessages() const
{
    return errors.size() > 0 || warnings.size() > 0;
}
