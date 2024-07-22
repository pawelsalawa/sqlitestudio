#include "sqlquerymodelcolumn.h"
#include "iconmanager.h"
#include <QDebug>

SqlQueryModelColumn::SqlQueryModelColumn(const QueryExecutor::ResultColumnPtr& resultColumn)
{
    displayName = resultColumn->displayName;
    column = resultColumn->column;
    alias = resultColumn->alias;
    table = resultColumn->table;
    tableAlias = resultColumn->tableAlias;
    database = resultColumn->database.isEmpty() ? "main": resultColumn->database;
    for (QueryExecutor::ColumnEditionForbiddenReason reason : resultColumn->editionForbiddenReasons)
        editionForbiddenReason << SqlQueryModelColumn::convert(reason);
}

SqlQueryModelColumn::~SqlQueryModelColumn()
{
    for (Constraint* constr : constraints)
        delete constr;

    constraints.clear();
}

void SqlQueryModelColumn::initMeta()
{
    qRegisterMetaType<SqlQueryModelColumn*>("SqlQueryModelColumn*");
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<SqlQueryModelColumn*>("SqlQueryModelColumn*");
#else
    // Qt 6 does it automatically
#endif
}

SqlQueryModelColumn::EditionForbiddenReason SqlQueryModelColumn::convert(QueryExecutor::EditionForbiddenReason reason)
{
    switch (reason)
    {
        case QueryExecutor::EditionForbiddenReason::NOT_A_SELECT:
            return EditionForbiddenReason::NOT_A_SELECT;
        case QueryExecutor::EditionForbiddenReason::SMART_EXECUTION_FAILED:
            return EditionForbiddenReason::SMART_EXECUTION_FAILED;
    }
    return static_cast<EditionForbiddenReason>(-1);
}

SqlQueryModelColumn::EditionForbiddenReason SqlQueryModelColumn::convert(QueryExecutor::ColumnEditionForbiddenReason reason)
{
    switch (reason)
    {
        case QueryExecutor::ColumnEditionForbiddenReason::EXPRESSION:
            return EditionForbiddenReason::EXPRESSION;
        case QueryExecutor::ColumnEditionForbiddenReason::SYSTEM_TABLE:
            return EditionForbiddenReason::SYSTEM_TABLE;
        case QueryExecutor::ColumnEditionForbiddenReason::COMPOUND_SELECT:
            return EditionForbiddenReason::COMPOUND_SELECT;
        case QueryExecutor::ColumnEditionForbiddenReason::GROUPED_RESULTS:
            return EditionForbiddenReason::GROUPED_RESULTS;
        case QueryExecutor::ColumnEditionForbiddenReason::DISTINCT_RESULTS:
            return EditionForbiddenReason::DISTINCT_RESULTS;
        case QueryExecutor::ColumnEditionForbiddenReason::COMM_TAB_EXPR:
            return EditionForbiddenReason::COMMON_TABLE_EXPRESSION;
        case QueryExecutor::ColumnEditionForbiddenReason::VIEW_NOT_EXPANDED:
            return EditionForbiddenReason::VIEW_NOT_EXPANDED;
    }
    return static_cast<EditionForbiddenReason>(-1);
}

QString SqlQueryModelColumn::resolveMessage(SqlQueryModelColumn::EditionForbiddenReason reason)
{
    switch (reason)
    {
        case EditionForbiddenReason::COMPOUND_SELECT:
            return QObject::tr("Cannot edit columns that are result of compound %1 statements (one that includes %2, %3 or %4 keywords).")
                    .arg("SELECT", "UNION", "INTERSECT", "EXCEPT");
        case EditionForbiddenReason::SMART_EXECUTION_FAILED:
            return QObject::tr("The query execution mechanism had problems with extracting ROWID's properly. This might be a bug in the application. You may want to report this.");
        case EditionForbiddenReason::EXPRESSION:
            return QObject::tr("Requested column is a result of SQL expression, instead of a simple column selection. Such columns cannot be edited.");
        case EditionForbiddenReason::SYSTEM_TABLE:
            return QObject::tr("Requested column belongs to restricted SQLite table. Those tables cannot be edited directly.");
        case EditionForbiddenReason::NOT_A_SELECT:
            return QObject::tr("Cannot edit results of query other than %1.").arg("SELECT");
        case EditionForbiddenReason::GROUPED_RESULTS:
            return QObject::tr("Cannot edit columns that are result of aggregated %1 statements.").arg("SELECT");
        case EditionForbiddenReason::DISTINCT_RESULTS:
            return QObject::tr("Cannot edit columns that are result of %1 statement.").arg("SELECT DISTINCT");
        case EditionForbiddenReason::COMMON_TABLE_EXPRESSION:
            return QObject::tr("Cannot edit columns that are result of common table expression statement (%1).").arg("WITH ... SELECT ...");
        case EditionForbiddenReason::GENERATED_COLUMN:
            return QObject::tr("Cannot edit table generated columns.");
        case EditionForbiddenReason::VIEW_NOT_EXPANDED:
            return QObject::tr("Cannot edit columns that are result of a view if the executed query reads from any multilevel views (i.e. a view that queries another view).");
    }
    qCritical() << "Reached null text message for SqlQueryModel::EditionForbiddenReason. This should not happen!";
    return QString();
}

void SqlQueryModelColumn::postProcessConstraints()
{
    if (isGenerated())
        editionForbiddenReason << EditionForbiddenReason::GENERATED_COLUMN;
}

bool SqlQueryModelColumn::isNumeric()
{
    return dataType.isNumeric();
}

bool SqlQueryModelColumn::isNull()
{
    return dataType.isNull();
}

bool SqlQueryModelColumn::canEdit()
{
    return editionForbiddenReason.size() == 0;
}

QString SqlQueryModelColumn::getEditionForbiddenReason()
{
    if (canEdit())
        return QString();

    // We sort reasons to get most significant reason at first position.
    QList<EditionForbiddenReason> list = editionForbiddenReason.values();
    std::sort(list.begin(), list.end());
    return resolveMessage(list[0]);
}

bool SqlQueryModelColumn::isPk() const
{
    return getConstraints<ConstraintPk*>().size() > 0;
}

bool SqlQueryModelColumn::isRowIdPk() const
{
    if (dataType.getType() != DataType::INTEGER)
        return false;

    for (ConstraintPk* pk : getConstraints<ConstraintPk*>())
        if (pk->scope == Constraint::Scope::COLUMN)
            return true;

    return false;
}

bool SqlQueryModelColumn::isAutoIncr() const
{
    for (ConstraintPk* pk : getConstraints<ConstraintPk*>())
        if (pk->autoIncrement)
            return true;

    return false;
}

bool SqlQueryModelColumn::isNotNull() const
{
    return getConstraints<ConstraintNotNull*>().size() > 0;
}

bool SqlQueryModelColumn::isUnique() const
{
    return getConstraints<ConstraintUnique*>().size() > 0;
}

bool SqlQueryModelColumn::isFk() const
{
    return getConstraints<ConstraintFk*>().size() > 0;
}

bool SqlQueryModelColumn::isDefault() const
{
    return getConstraints<ConstraintDefault*>().size() > 0;
}

bool SqlQueryModelColumn::isCollate() const
{
    return getConstraints<ConstraintCollate*>().size() > 0;
}

bool SqlQueryModelColumn::isGenerated() const
{
    return getConstraints<ConstraintGenerated*>().size() > 0;
}

QString SqlQueryModelColumn::getAliasedName() const
{
    return this->alias.isEmpty() ? this->column : this->alias;
}

QList<SqlQueryModelColumn::ConstraintFk*> SqlQueryModelColumn::getFkConstraints() const
{
    return getConstraints<ConstraintFk*>();
}

SqlQueryModelColumn::ConstraintDefault* SqlQueryModelColumn::getDefaultConstraint() const
{
    QList<ConstraintDefault*> list = getConstraints<ConstraintDefault*>();
    if (list.size() == 0)
        return nullptr;

    return list[0];
}

AliasedTable SqlQueryModelColumn::getAliasedTable() const
{
    return AliasedTable(database, table, tableAlias);
}

TYPE_OF_QHASH qHash(SqlQueryModelColumn::EditionForbiddenReason reason)
{
    return static_cast<int>(reason);
}

QDataStream&operator <<(QDataStream& out, const SqlQueryModelColumn* col)
{
    out << reinterpret_cast<quint64>(col);
    return out;
}

QDataStream&operator >>(QDataStream& in, SqlQueryModelColumn*& col)
{
    quint64 ptr;
    in >> ptr;
    col = reinterpret_cast<SqlQueryModelColumn*>(ptr);
    return in;
}


SqlQueryModelColumn::Constraint* SqlQueryModelColumn::Constraint::create(const QString& column, SqliteCreateTable::ConstraintPtr tableConstraint)
{
    return create(column, tableConstraint.data());
}

SqlQueryModelColumn::Constraint* SqlQueryModelColumn::Constraint::create(const QString& column, SqliteCreateTable::Constraint* tableConstraint)
{
    Constraint* constr = nullptr;
    switch (tableConstraint->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        {
            if (!tableConstraint->doesAffectColumn(column))
                return nullptr;

            constr = new ConstraintPk();
            constr->type = Type::PRIMARY_KEY;
            dynamic_cast<ConstraintPk*>(constr)->multiColumns =
                map<SqliteIndexedColumn*, QString>(tableConstraint->indexedColumns, [](SqliteIndexedColumn* idxCol) -> QString
                {
                    return idxCol->detokenize().trimmed();
                });
            break;
        }
        case SqliteCreateTable::Constraint::UNIQUE:
        {
            if (!tableConstraint->doesAffectColumn(column))
                return nullptr;

            constr = new ConstraintUnique();
            constr->type = Type::UNIQUE;
            break;
        }
        case SqliteCreateTable::Constraint::CHECK:
        {
            ConstraintCheck* check = new ConstraintCheck();
            check->condition = tableConstraint->expr->detokenize();
            constr = check;
            constr->type = Type::CHECK;
            break;
        }
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
        {
            int idx = tableConstraint->getAffectedColumnIdx(column);
            if (idx < 0 || tableConstraint->foreignKey->indexedColumns.size() <= idx)
            {
                // This case is perfectly fine if there are for example 2 foreign keys on the table,
                // for 2 different columns. For each of those columns there will be 1 FK
                // that enters here.
                //qWarning() << "Could not find FK column for definition:" << tableConstraint->detokenize();
                return nullptr;
            }

            ConstraintFk* fk = new ConstraintFk();
            fk->foreignTable = tableConstraint->foreignKey->foreignTable;
            fk->foreignColumn = tableConstraint->foreignKey->indexedColumns[idx]->name;

            constr = fk;
            constr->type = Type::FOREIGN_KEY;
            break;
        }
        default:
            return nullptr;
    }

    constr->scope = Scope::TABLE;
    constr->definition = tableConstraint->detokenize();
    return constr;
}

SqlQueryModelColumn::Constraint* SqlQueryModelColumn::Constraint::create(SqliteCreateTable::Column::ConstraintPtr columnConstraint)
{
    return create(columnConstraint.data());
}

SqlQueryModelColumn::Constraint* SqlQueryModelColumn::Constraint::create(SqliteCreateTable::Column::Constraint* columnConstraint)
{
    Constraint* constr = nullptr;
    switch (columnConstraint->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
        {
            ConstraintPk* pk = new ConstraintPk();
            pk->autoIncrement = columnConstraint->autoincrKw;
            constr = pk;
            constr->type = Type::PRIMARY_KEY;
            break;
        }
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
        {
            constr = new ConstraintNotNull();
            constr->type = Type::NOT_NULL;
            break;
        }
        case SqliteCreateTable::Column::Constraint::UNIQUE:
        {
            constr = new ConstraintUnique();
            constr->type = Type::UNIQUE;
            break;
        }
        case SqliteCreateTable::Column::Constraint::CHECK:
        {
            ConstraintCheck* check = new ConstraintCheck();
            check->condition = columnConstraint->expr->detokenize();
            constr = check;
            constr->type = Type::CHECK;
            break;
        }
        case SqliteCreateTable::Column::Constraint::DEFAULT:
        {
            ConstraintDefault* def = new ConstraintDefault();
            if (!columnConstraint->id.isNull())
                def->defaultValue = columnConstraint->id;
            else if (!columnConstraint->ctime.isNull())
                def->defaultValue = columnConstraint->ctime;
            else if (columnConstraint->expr)
                def->defaultValue = columnConstraint->expr->detokenize();
            else
                def->defaultValue = columnConstraint->literalValue.toString();

            constr = def;
            constr->type = Type::DEFAULT;
            break;
        }
        case SqliteCreateTable::Column::Constraint::COLLATE:
        {
            ConstraintCollate* collate = new ConstraintCollate();
            collate->collationName = columnConstraint->collationName;
            constr = collate;
            constr->type = Type::COLLATE;
            break;
        }
        case SqliteCreateTable::Column::Constraint::GENERATED:
        {
            ConstraintGenerated* generate = new ConstraintGenerated();
            generate->generatedType = columnConstraint->generatedType;
            generate->expr = columnConstraint->expr ? columnConstraint->expr->detokenize() : QString();
            constr = generate;
            constr->type = Type::GENERATED;
            break;
        }
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
        {
            if (columnConstraint->foreignKey->indexedColumns.size() == 0)
            {
                qWarning() << "No foreign column defined for FK column constraint while creating SqlQueryModelColumn::Constraint.";
                return nullptr;
            }

            ConstraintFk* fk = new ConstraintFk();
            fk->foreignTable = columnConstraint->foreignKey->foreignTable;
            fk->foreignColumn = columnConstraint->foreignKey->indexedColumns.first()->name;

            constr = fk;
            constr->type = Type::FOREIGN_KEY;
            break;
        }
        default:
            return nullptr;
    }

    constr->scope = Scope::COLUMN;
    constr->definition = columnConstraint->detokenize();
    return constr;
}

template <class T>
QList<T> SqlQueryModelColumn::getConstraints() const
{
    QList<T> results;
    for (Constraint* constr : constraints)
        if (dynamic_cast<T>(constr))
            results << dynamic_cast<T>(constr);

    return results;
}


QString SqlQueryModelColumn::ConstraintPk::getTypeString() const
{
    return "PRIMARY KEY";
}

QString SqlQueryModelColumn::ConstraintPk::getDetails() const
{
    QStringList detailList;
    if (!multiColumns.isEmpty())
        detailList << "("+multiColumns.join(", ")+")";

    if (autoIncrement)
        detailList << "AUTOINCREMENT";

    if (onConflict != SqliteConflictAlgo::null)
        detailList << QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict));

    if (detailList.size() > 0)
    {
        if (detailList.size() > 1)
            return "("+detailList.join(", ")+")";
        else
            return detailList.join(", ");
    }

    return "";
}

Icon* SqlQueryModelColumn::ConstraintPk::getIcon() const
{
    return ICONS.CONSTRAINT_PRIMARY_KEY;
}

QString SqlQueryModelColumn::ConstraintFk::getTypeString() const
{
    return "FOREIGN KEY";
}

QString SqlQueryModelColumn::ConstraintFk::getDetails() const
{
    return "("+QObject::tr("references table %1, column %2", "data view tooltip").arg(foreignTable).arg(foreignColumn)+")";
}

Icon* SqlQueryModelColumn::ConstraintFk::getIcon() const
{
    return ICONS.CONSTRAINT_FOREIGN_KEY;
}

QString SqlQueryModelColumn::ConstraintUnique::getTypeString() const
{
    return "UNIQUE";
}

QString SqlQueryModelColumn::ConstraintUnique::getDetails() const
{
    if (onConflict != SqliteConflictAlgo::null)
        return "("+QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict))+")";

    return QString();
}

Icon* SqlQueryModelColumn::ConstraintUnique::getIcon() const
{
    return ICONS.CONSTRAINT_UNIQUE;
}

QString SqlQueryModelColumn::ConstraintNotNull::getTypeString() const
{
    return "NOT NULL";
}

QString SqlQueryModelColumn::ConstraintNotNull::getDetails() const
{
    if (onConflict != SqliteConflictAlgo::null)
        return "("+QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict))+")";

    return QString();
}

Icon* SqlQueryModelColumn::ConstraintNotNull::getIcon() const
{
    return ICONS.CONSTRAINT_NOT_NULL;
}

QString SqlQueryModelColumn::ConstraintDefault::getTypeString() const
{
    return "DEFAULT";
}

QString SqlQueryModelColumn::ConstraintDefault::getDetails() const
{
    return "("+defaultValue+")";
}

Icon* SqlQueryModelColumn::ConstraintDefault::getIcon() const
{
    return ICONS.CONSTRAINT_DEFAULT;
}

QString SqlQueryModelColumn::ConstraintCheck::getTypeString() const
{
    return "CHECK";
}

QString SqlQueryModelColumn::ConstraintCheck::getDetails() const
{
    QStringList detailList;
    detailList << QObject::tr("condition: %1", "data view tooltip").arg(condition);

    if (onConflict != SqliteConflictAlgo::null)
        detailList << QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict));

    return "("+detailList.join(", ")+")";
}

Icon* SqlQueryModelColumn::ConstraintCheck::getIcon() const
{
    return ICONS.CONSTRAINT_CHECK;
}

QString SqlQueryModelColumn::ConstraintCollate::getTypeString() const
{
    return "COLLATE";
}

QString SqlQueryModelColumn::ConstraintCollate::getDetails() const
{
    return "("+QObject::tr("collation name: %1", "data view tooltip").arg(collationName)+")";
}

Icon* SqlQueryModelColumn::ConstraintCollate::getIcon() const
{
    return ICONS.CONSTRAINT_COLLATION;
}

QString SqlQueryModelColumn::ConstraintGenerated::getTypeString() const
{
    return "GENERATED";
}

QString SqlQueryModelColumn::ConstraintGenerated::getDetails() const
{
    return QString("(%1) %2").arg(expr, SqliteCreateTable::Column::Constraint::toString(generatedType)).trimmed();
}

Icon* SqlQueryModelColumn::ConstraintGenerated::getIcon() const
{
    return generatedType == SqliteCreateTable::Column::Constraint::GeneratedType::STORED ?
                ICONS.CONSTRAINT_GENERATED_STORED : ICONS.CONSTRAINT_GENERATED_VIRTUAL;
}
