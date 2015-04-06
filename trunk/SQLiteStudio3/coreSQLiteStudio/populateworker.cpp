#include "populateworker.h"
#include "common/utils_sql.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include "plugins/populateplugin.h"
#include "services/notifymanager.h"

PopulateWorker::PopulateWorker(Db* db, const QString& table, const QStringList& columns, const QList<PopulateEngine*>& engines, qint64 rows, QObject* parent) :
    QObject(parent), db(db), table(table), columns(columns), engines(engines), rows(rows)
{
}

PopulateWorker::~PopulateWorker()
{
}

void PopulateWorker::run()
{
    static const QString insertSql = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3);");

    if (!db->begin())
    {
        notifyError(tr("Could not start transaction in order to perform table populating. Error details: %1").arg(db->getErrorText()));
        emit finished(false);
        return;
    }

    Dialect dialect = db->getDialect();
    QString wrappedTable = wrapObjIfNeeded(table, dialect);

    QStringList cols;
    QStringList argList;
    for (const QString& column : columns)
    {
        cols << wrapObjIfNeeded(column, dialect);
        argList << "?";
    }

    QString finalSql = insertSql.arg(wrappedTable, cols.join(", "), argList.join(", "));
    SqlQueryPtr query = db->prepare(finalSql);

    QList<QVariant> args;
    bool nextValueError = false;
    for (qint64 i = 0; i < rows; i++)
    {
        if (i == 0 && !beforePopulating())
            return;

        args.clear();
        for (PopulateEngine* engine : engines)
            args << engine->nextValue(nextValueError);

        query->setArgs(args);
        if (!query->execute())
        {
            notifyError(tr("Error while populating table: %1").arg(query->getErrorText()));
            db->rollback();
            emit finished(false);
            return;
        }

        emit finishedStep(i + 1);
    }

    if (!db->commit())
    {
        notifyError(tr("Could not commit transaction after table populating. Error details: %1").arg(db->getErrorText()));
        db->rollback();
        emit finished(false);
        return;
    }

    afterPopulating();
    emit finished(true);
}

bool PopulateWorker::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}

bool PopulateWorker::beforePopulating()
{
    for (PopulateEngine* engine : engines)
    {
        if (!engine->beforePopulating(db, table))
        {
            db->rollback();
            emit finished(false);
            return false;
        }
    }
    return true;
}

void PopulateWorker::afterPopulating()
{
    for (PopulateEngine* engine : engines)
        engine->afterPopulating();
}

void PopulateWorker::interrupt()
{
    QMutexLocker locker(&interruptMutex);
    interrupted = true;
}
