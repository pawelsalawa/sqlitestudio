#include "sqlquerymodelcolumn.h"
#include <QDebug>

SqlQueryModelColumn::SqlQueryModelColumn(const QueryExecutor::ResultColumnPtr& resultColumn)
{
    displayName = resultColumn->displayName;
    column = resultColumn->column;
    table = resultColumn->table;
    database = resultColumn->database.isEmpty() ? "main": resultColumn->database;
    foreach (QueryExecutor::ColumnEditionForbiddenReason reason, resultColumn->editionForbiddenReasons)
        editionForbiddenReason << SqlQueryModelColumn::convert(reason);
}

SqlQueryModelColumn::~SqlQueryModelColumn()
{
    foreach (Constraint* constr, constraints)
        delete constr;

    constraints.clear();
}

void SqlQueryModelColumn::initMeta()
{
    qRegisterMetaType<SqlQueryModelColumn*>("SqlQueryModelColumn*");
    qRegisterMetaTypeStreamOperators<SqlQueryModelColumn*>("SqlQueryModelColumn*");
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
    }
    return static_cast<EditionForbiddenReason>(-1);
}

QString SqlQueryModelColumn::resolveMessage(SqlQueryModelColumn::EditionForbiddenReason reason)
{
    switch (reason)
    {
        case EditionForbiddenReason::COMPOUND_SELECT:
            return QObject::tr("Cannot edit columns that are result of compound SELECT statements (one that includes UNION, INTERSECT or EXCEPT keywords).");
        case EditionForbiddenReason::SMART_EXECUTION_FAILED:
            return QObject::tr("The query execution mechanism had problems with extracting ROWID's properly. This might be a bug in the application. You may want to report this.");
        case EditionForbiddenReason::EXPRESSION:
            return QObject::tr("Requested column is a result of SQL expression, instead of a simple column selection. Such columns cannot be edited.");
        case EditionForbiddenReason::SYSTEM_TABLE:
            return QObject::tr("Requested column belongs to restricted SQLite table. Those tables cannot be edited directly.");
        case EditionForbiddenReason::NOT_A_SELECT:
            return QObject::tr("Cannot edit results of query other than SELECT.");
        case EditionForbiddenReason::GROUPED_RESULTS:
            return QObject::tr("Cannot edit columns that are result of aggregated SELECT statements.");
        case EditionForbiddenReason::DISTINCT_RESULTS:
            return QObject::tr("Cannot edit columns that are result of SELECT DISTINCT statement.");
        case EditionForbiddenReason::COMMON_TABLE_EXPRESSION:
            return QObject::tr("Cannot edit columns that are result of common table expression statement (%1).").arg("WITH ... SELECT ...");
    }
    qCritical() << "Reached null text message for SqlQueryModel::EditionForbiddenReason. This should not happen!";
    return QString::null;
}

bool SqlQueryModelColumn::isNumeric()
{
    return dataType.type && ::DataType::isNumeric(dataType.type);
}

bool SqlQueryModelColumn::canEdit()
{
    return editionForbiddenReason.size() == 0;
}

QString SqlQueryModelColumn::getEditionForbiddenReason()
{
    if (canEdit())
        return QString::null;

    // We sort reasons to get most significant reason at first position.
    QList<EditionForbiddenReason> list = editionForbiddenReason.toList();
    qSort(list);
    return resolveMessage(list[0]);
}

bool SqlQueryModelColumn::isPk() const
{
    return getConstraints<ConstraintPk*>().size() > 0;
}

bool SqlQueryModelColumn::isAutoIncr() const
{
    foreach (ConstraintPk* pk, getConstraints<ConstraintPk*>())
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

SqlQueryModelColumn::DataType::DataType()
{
}

SqlQueryModelColumn::DataType::DataType(const QString& type, const QVariant& precision, const QVariant& scale)
{
    this->type = ::DataType::fromString(type, Qt::CaseInsensitive);
    this->typeStr = type;
    this->precision = precision;
    this->scale = scale;
}

SqlQueryModelColumn::DataType::DataType(const SqlQueryModelColumn::DataType& other)
{
    this->type = other.type;
    this->typeStr = other.typeStr;
    this->precision = other.precision;
    this->scale = scale;
}

void SqlQueryModelColumn::DataType::setEmpty()
{
    type = ::DataType::_NULL;
    typeStr = "";
}

QString SqlQueryModelColumn::DataType::toString()
{
    QString str = typeStr;
    if (!precision.isNull())
    {
        if (!scale.isNull())
            str += " ("+precision.toString()+", "+scale.toString()+")";
        else
            str += " ("+precision.toString()+")";
    }
    return str;
}

int qHash(SqlQueryModelColumn::EditionForbiddenReason reason)
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
    Constraint* constr = nullptr;
    switch (tableConstraint->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        {
            if (!tableConstraint->doesAffectColumn(column))
                return nullptr;

            constr = new ConstraintPk();
            constr->type = Type::PRIMARY_KEY;
            break;
        }
        case SqliteCreateTable::Constraint::UNIQUE:
        {
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
                qWarning() << "Could not find FK column for definition:" << tableConstraint->detokenize();
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
    foreach (Constraint* constr, constraints)
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
    if (autoIncrement)
        detailList << "AUTOINCREMENT";

    if (onConflict != SqliteConflictAlgo::null)
        detailList << QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict));

    if (detailList.size() > 0)
        return "("+detailList.join(", ")+")";

    return "";
}

QString SqlQueryModelColumn::ConstraintPk::getIconName() const
{
    return "pk";
}

QString SqlQueryModelColumn::ConstraintFk::getTypeString() const
{
    return "FOREIGN KEY";
}

QString SqlQueryModelColumn::ConstraintFk::getDetails() const
{
    return "("+QObject::tr("references table %1, column %2", "data view tooltip").arg(foreignTable).arg(foreignColumn)+")";
}

QString SqlQueryModelColumn::ConstraintFk::getIconName() const
{
    return "fk";
}

QString SqlQueryModelColumn::ConstraintUnique::getTypeString() const
{
    return "UNIQUE";
}

QString SqlQueryModelColumn::ConstraintUnique::getDetails() const
{
    if (onConflict != SqliteConflictAlgo::null)
        return "("+QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict))+")";

    return QString::null;
}

QString SqlQueryModelColumn::ConstraintUnique::getIconName() const
{
    return "unique";
}

QString SqlQueryModelColumn::ConstraintNotNull::getTypeString() const
{
    return "NOT NULL";
}

QString SqlQueryModelColumn::ConstraintNotNull::getDetails() const
{
    if (onConflict != SqliteConflictAlgo::null)
        return "("+QObject::tr("on conflict: %1", "data view tooltip").arg(sqliteConflictAlgo(onConflict))+")";

    return QString::null;
}

QString SqlQueryModelColumn::ConstraintNotNull::getIconName() const
{
    return "not_null";
}

QString SqlQueryModelColumn::ConstraintDefault::getTypeString() const
{
    return "DEFAULT";
}

QString SqlQueryModelColumn::ConstraintDefault::getDetails() const
{
    return "("+defaultValue+")";
}

QString SqlQueryModelColumn::ConstraintDefault::getIconName() const
{
    return "default";
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

QString SqlQueryModelColumn::ConstraintCheck::getIconName() const
{
    return "check";
}

QString SqlQueryModelColumn::ConstraintCollate::getTypeString() const
{
    return "COLLATE";
}

QString SqlQueryModelColumn::ConstraintCollate::getDetails() const
{
    return "("+QObject::tr("collation name: %1", "data view tooltip").arg(collationName)+")";
}

QString SqlQueryModelColumn::ConstraintCollate::getIconName() const
{
    return "collation";
}
