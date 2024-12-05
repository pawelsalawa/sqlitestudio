#include "queryexecutorsmarthints.h"
#include "parser/ast/sqlitecreatetable.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"

bool QueryExecutorSmartHints::exec()
{
    for (SqliteQueryPtr& query : context->parsedQueries)
    {
        checkForFkDataTypeMismatch(query);
    }
    return true;
}

void QueryExecutorSmartHints::checkForFkDataTypeMismatch(const SqliteQueryPtr& query) {
    if (query->queryType != SqliteQueryType::CreateTable)
        return;

    SchemaResolver resolver(db);

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    StrHash<StrHash<DataType>> fkTableColumnTypes;
    StrHash<DataType> localColumnTypes;
    for (SqliteCreateTable::Column*& col : createTable->columns)
    {
        DataType localColumnType = col->type ? col->type->toDataType() : DataType();
        localColumnTypes[col->name] = localColumnType;

        for (SqliteCreateTable::Column::Constraint*& constr : col->getConstraints(SqliteCreateTable::Column::Constraint::FOREIGN_KEY))
        {
            if (constr->foreignKey->indexedColumns.isEmpty())
                continue;

            QString fkTable = constr->foreignKey->foreignTable;
            QString fkColumn = constr->foreignKey->indexedColumns.first()->name;

            if (!fkTableColumnTypes.contains(fkTable, Qt::CaseInsensitive))
                fkTableColumnTypes[fkTable] = resolver.getTableColumnDataTypesByName(fkTable);

            StrHash<DataType> fkTypes = fkTableColumnTypes.value(fkTable, Qt::CaseInsensitive);
            DataType fkType = fkTypes.value(fkColumn, Qt::CaseInsensitive);

            checkForFkDataTypeMismatch(createTable->table, col->name, localColumnType, fkTable, fkColumn, fkType);
        }
    }

    for (SqliteCreateTable::Constraint*& constr : createTable->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY))
    {
        QList<SqliteIndexedColumn*> localColumns = constr->indexedColumns;
        QList<SqliteIndexedColumn*> fkColumns = constr->foreignKey->indexedColumns;
        QListIterator<SqliteIndexedColumn*> localColumnsIt(localColumns);
        QListIterator<SqliteIndexedColumn*> fkColumnsIt(fkColumns);

        QString fkTable = constr->foreignKey->foreignTable;
        if (!fkTableColumnTypes.contains(fkTable, Qt::CaseInsensitive))
            fkTableColumnTypes[fkTable] = resolver.getTableColumnDataTypesByName(fkTable);

        StrHash<DataType> fkTypes = fkTableColumnTypes.value(fkTable, Qt::CaseInsensitive);

        while (localColumnsIt.hasNext() && fkColumnsIt.hasNext())
        {
            QString localColumn = localColumnsIt.next()->name;
            QString fkColumn = fkColumnsIt.next()->name;

            DataType localType = localColumnTypes.value(localColumn, Qt::CaseInsensitive);
            DataType fkType = fkTypes.value(fkColumn, Qt::CaseInsensitive);

            checkForFkDataTypeMismatch(createTable->table, localColumn, localType, fkTable, fkColumn, fkType);
        }
    }
}

void QueryExecutorSmartHints::checkForFkDataTypeMismatch(const QString &localTable, const QString &localColumn, const DataType &localType, const QString &fkTable, const QString &fkColumn, const DataType &fkType)
{
    if (localType.toString().toLower().trimmed() != fkType.toString().toLower().trimmed())
    {
        notifyWarn(tr("Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.")
                       .arg(localColumn, localTable, fkColumn, fkTable, localType.toString(), fkType.toString()));
    }
}
