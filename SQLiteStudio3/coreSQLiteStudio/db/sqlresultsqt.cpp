#include "sqlresultsqt.h"
#include "sqlresultsrowqt.h"
#include "parser/parser.h"
#include "schemaresolver.h"
#include "db/db.h"
#include <QListIterator>
#include <QSqlRecord>
#include <QSqlResult>
#include <QSqlError>

SqlResultsQt::SqlResultsQt()
{
}

SqlResultsQt::SqlResultsQt(QSqlQuery &query, Db* db)
{
    this->query = query;
    this->query.next();
    this->db = db;

    bool ok;
    quint64 id = query.lastInsertId().toLongLong(&ok);
    if (ok)
        insertRowId["ROWID"] = id;

    readColumns();
}

SqlResultsQt::SqlResultsQt(QSqlQuery &query, Db* db, const QList<QVariant> &args) :
    SqlResultsQt(query, db)
{
    handleInsert(args);
}

SqlResultsQt::SqlResultsQt(QSqlQuery &query, Db* db, const QHash<QString, QVariant> &args) :
    SqlResultsQt(query, db)
{
    handleInsert(args);
}

SqlResultsQt::~SqlResultsQt()
{
}

void SqlResultsQt::setLastInsertRowId(const RowId& rowId)
{
    insertRowId = rowId;
}

SqlResultsRowPtr SqlResultsQt::next()
{
    if (!hasNext())
        return SqlResultsRowPtr();

    SqlResultsRowQt* row = new SqlResultsRowQt();
    for (int i = 0; i < columns.size(); i++)
        row->setValue(columns[i], query.value(i));

    query.next();

    return SqlResultsRowPtr(row);
}

bool SqlResultsQt::hasNext()
{
    return query.isValid();
}

QString SqlResultsQt::getErrorText()
{
    return query.lastError().databaseText().isEmpty() ? query.lastError().driverText() : query.lastError().databaseText();
}

int SqlResultsQt::getErrorCode()
{
    return query.lastError().number();
}

QStringList SqlResultsQt::getColumnNames()
{
    return columns;
}

int SqlResultsQt::columnCount()
{
    return columns.size();
}

qint64 SqlResultsQt::rowCount()
{
    return query.size();
}

qint64 SqlResultsQt::rowsAffected()
{
    qint64 rows = query.numRowsAffected();
    if (rows >= 0)
        return rows;

    return 0;
}

RowId SqlResultsQt::getInsertRowId()
{
    updateRowIdForInsert();
    return insertRowId;
}

void SqlResultsQt::restart()
{
    query.seek(-1);
}

void SqlResultsQt::readColumns()
{
    if (!query.isValid())
        return;

    QSqlRecord record = query.record();
    int totalFields = record.count();
    for (int i = 0; i < totalFields; i++)
        columns << record.fieldName(i);
}

void SqlResultsQt::handleInsert(const QList<QVariant> &args)
{
    extractInsert();
    if (!insertStmt)
        return;

    insertArgList = args;
}

void SqlResultsQt::handleInsert(const QHash<QString, QVariant> &args)
{
    extractInsert();
    if (!insertStmt)
        return;

    insertArgHash = args;
}

void SqlResultsQt::extractInsert()
{
    Parser parser(db->getDialect());
    insertStmt = parser.parse<SqliteInsert>(query.lastQuery());
}

void SqlResultsQt::updateRowIdForInsert()
{
    if (!insertStmt || insertRowId.size() > 0) // not an INSERT, or we already have ROWID
        return;

    if (insertArgHash.size() > 0)
        insertRowId = getRowId(query.lastQuery(), insertArgHash);
    else
        insertRowId = getRowId(query.lastQuery(), insertArgList);
}

RowId SqlResultsQt::getRowId(const QString& query, const QList<QVariant>& args)
{
    Parser parser(db->getDialect());
    SqliteInsertPtr insert = parser.parse<SqliteInsert>(query);
    if (!insert)
        return RowId();

    SchemaResolver resolver(db);
    QStringList pkColumns = resolver.getWithoutRowIdTableColumns(insert->database, insert->table);
    if (pkColumns.size() == 0)
        return RowId();

    RowId rowId;
    int colPos = 0;
    foreach (const QString& insertColumn, insert->columnNames)
    {
        if (pkColumns.contains(insertColumn, Qt::CaseInsensitive))
        {
            if (args[colPos].isNull())
                return RowId(); // passed null for PK value, which causes usage of DEFAULT constraint and we cannot always predict it's value.

            rowId[insertColumn] = args[colPos];
        }

        colPos++;
    }
    return rowId;
}

RowId SqlResultsQt::getRowId(const QString& query, const QHash<QString, QVariant>& args)
{
    Parser parser(db->getDialect());
    SqliteInsertPtr insert = parser.parse<SqliteInsert>(query);
    if (!insert)
        return RowId();

    SchemaResolver resolver(db);
    QStringList pkColumns = resolver.getWithoutRowIdTableColumns(insert->database, insert->table);
    if (pkColumns.size() == 0)
        return RowId();

    // We need make a translation of lower-case keys from QHash into real keys,
    // so we can query the hash by lower-case keys.
    QHash<QString, QVariant> lowerArgs;
    QHashIterator<QString, QVariant> it(args);
    while (it.hasNext())
    {
        it.next();
        lowerArgs[it.key().toLower()] = it.value();
    }

    RowId rowId;
    QString lowerColumn;
    foreach (const QString& insertColumn, insert->columnNames)
    {
        lowerColumn = insertColumn.toLower();
        if (pkColumns.contains(insertColumn, Qt::CaseInsensitive))
        {
            if (!lowerArgs.contains(lowerColumn))
                return RowId(); // column is part of PK, but is not in arguments, we cannot collect such RowId

            if (lowerArgs[lowerColumn].isNull())
                return RowId(); // passed null for PK value, which causes usage of DEFAULT constraint and we cannot always predict it's value.

            rowId[insertColumn] = lowerArgs[lowerColumn];
        }
    }
    return rowId;
}
