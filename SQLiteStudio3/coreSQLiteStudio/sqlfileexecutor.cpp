#include "sqlfileexecutor.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
// #include "services/config.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>

SqlFileExecutor::SqlFileExecutor(QObject *parent)
    : QObject{parent}
{
}

void SqlFileExecutor::execSqlFromFile(Db* db, const QString& filePath, bool ignoreErrors, QString codec, bool async)
{
    if (!db || !db->isOpen())
    {
        emit execEnded();
        return;
    }

    if (executionInProgress)
    {
        emit execEnded();
        return;
    }

    // Initially the #4871 was caused by this FK disabling, but when this code is commented out,
    // then sample file from #5395     // won't execute (order of insertions requires disabled FK).
    // The thing is that FK cannot be toggled on/off when there is an active transaction (or savepoint).
    // That was the initial root cause in #4871, as the FK state was restored (attempted to)
    // before the transaction was commited/rolled back. It's fixed now.
    fkWasEnabled = db->exec("PRAGMA foreign_keys")->getSingleCell().toBool();
    if (fkWasEnabled)
    {
        SqlQueryPtr res = db->exec("PRAGMA foreign_keys = 0");
        if (res->isError())
        {
            qDebug() << "Failed to temporarily disable foreign keys enforcement:" << db->getErrorText();
            emit execEnded();
            return;
        }
    }

    // Exec file
    executionInProgress = 1;
    this->ignoreErrors = ignoreErrors;
    this->codec = codec;
    this->filePath = filePath;
    this->db = db;
    emit updateProgress(0);
    txName = db->beginNamed();
    if (txName.isNull())
    {
        notifyError(tr("Could not execute SQL, because application has failed to start transaction: %1").arg(db->getErrorText()));
        emit execEnded();
        return;
    }

    if (async)
        runInThread([=, this]{ execInThread(); });
    else
        execInThread();
}

bool SqlFileExecutor::isExecuting() const
{
    return executionInProgress;
}

void SqlFileExecutor::stopExecution()
{
    if (!executionInProgress)
    {
        emit execEnded();
        return;
    }

    executionInProgress = 0;

    if (db) // should always be there, but just in case
    {
        db->interrupt();
        rollback();
        db = nullptr;
        notifyWarn(tr("Execution from file cancelled. Any queries executed so far have been rolled back."));
    }
    emit execEnded();
}

bool SqlFileExecutor::execQueryFromFile(Db* db, const QString& sql)
{
    return !db->exec(sql)->isError();
}

void SqlFileExecutor::execInThread()
{
    // Open file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notifyError(tr("Could not open file '%1' for reading: %2").arg(filePath, file.errorString()));
        executionInProgress = 0;
        emit execEnded();
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(textEncodingForName(codec));

    qint64 fileSize = file.size();
    int attemptedExecutions = 0;
    int executed = 0;
    bool ok = true;

    QElapsedTimer timer;
    timer.start();
    QList<QPair<QString, QString>> errors = executeFromStream(stream, executed, attemptedExecutions, ok, fileSize);
    int millis = timer.elapsed();

    if (executionInProgress.loadAcquire())
    {
        handleExecutionResults(db, executed, attemptedExecutions, ok, ignoreErrors, millis);
        if (!errors.isEmpty())
            emit execErrors(errors, !ok && !ignoreErrors);
    }
    file.close();

    if (fkWasEnabled)
    {
        SqlQueryPtr res = db->exec("PRAGMA foreign_keys = 1");
        if (res->isError())
            qDebug() << "Failed to restore foreign keys enforcement after execution SQL from file:" << res->getErrorText();
    }

    emit execEnded();
    executionInProgress = 0;
}

void SqlFileExecutor::handleExecutionResults(Db* db, int executed, int attemptedExecutions, bool ok, bool ignoreErrors, int millis)
{
    bool doCommit = ok ? true : ignoreErrors;
    if (doCommit)
    {
        if (!commit())
        {
            notifyError(tr("Could not execute SQL, because application has failed to commit the transaction: %1").arg(db->getErrorText()));
            rollback();
        }
        else if (!ok) // committed with errors
        {
            notifyInfo(tr("Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.")
                           .arg(QString::number(executed), QString::number(millis / 1000.0), QString::number(attemptedExecutions - executed)));
            emit schemaNeedsRefreshing(db);
        }
        else
        {
            notifyInfo(tr("Finished executing %1 queries in %2 seconds.").arg(QString::number(executed), QString::number(millis / 1000.0)));
            emit schemaNeedsRefreshing(db);
        }
    }
    else
    {
        rollback();
        notifyError(tr("Could not execute SQL due to error."));
    }
}

QList<QPair<QString, QString>> SqlFileExecutor::executeFromStream(
        QTextStream& stream,
        int& executed,
        int& attemptedExecutions,
        bool& ok,
        qint64 fileSize)
{
    QList<QPair<QString, QString>> errors;
    qint64 pos = 0;
    QChar c;
    QString sql;
    sql.reserve(10000);
    SqlQueryPtr results;
    while (!stream.atEnd() && executionInProgress.loadAcquire())
    {
        while (!db->isComplete(sql) && !stream.atEnd())
        {
            stream >> c;
            sql.append(c);
            while (c != ';' && !stream.atEnd())
            {
                stream >> c;
                sql.append(c);
            }
        }

        sql = processDotCommands(sql, errors);

        if (shouldSkipQuery(sql, stream.atEnd()))
        {
            sql.clear();
            continue;
        }

        results = db->exec(sql);
        attemptedExecutions++;
        if (results->isError())
        {
            ok = false;
            errors << QPair<QString, QString>(sql, results->getErrorText());
            if (!ignoreErrors)
                break;
        }
        else
            executed++;

        sql.clear();
        if (attemptedExecutions % 100 == 0)
        {
            pos = stream.device()->pos();
            emit updateProgress(static_cast<int>(100 * pos / fileSize));
        }
    }
    return errors;
}

bool SqlFileExecutor::shouldSkipQuery(const QString& sql, bool isEnd) const
{
    if (sql.trimmed().isEmpty() || (!isEnd && !db->isComplete(sql)))
        return true;

    QString upper = sql.toUpper().trimmed();
    return (upper.startsWith("BEGIN") ||
            upper.startsWith("COMMIT") ||
            upper.startsWith("ROLLBACK") ||
            upper.startsWith("END"));
}

QString SqlFileExecutor::processDotCommands(const QString& sql, QList<QPair<QString, QString>>& errors)
{
    if (executionMode == STRICT || !sql.startsWith(".") && !sql.contains("\n."))
        return sql;

    QString content = removeComments(sql);
    if (content.trimmed().startsWith("."))
    {
        QStringList lines = content.split("\n");
        QStringList processedLines;
        for (const QString& line : lines)
        {
            if (line.trimmed().startsWith("."))
                handleDotCommand(line.trimmed(), errors);
            else
                processedLines << line;
        }
        content = processedLines.join("\n");
    }
    return content;
}

void SqlFileExecutor::handleDotCommand(const QString& cmdLine, QList<QPair<QString, QString>>& errors)
{
    if (executionMode == PERMISSIVE)
        return;

    QString line = cmdLine.trimmed();
    while (line.endsWith(';'))
        line.chop(1);

    QStringList tokens = splitArgs(line);
    QString cmd = tokens[0].mid(1).toLower(); // usuń '.'

    if (cmd == "print")
    {
        notifyInfo(tokens.mid(1).join(" "));
        return;
    }

    if (cmd == "read")
    {
        QString path = tokens.mid(1).join(" ");
        SqlFileExecutor nestedExecutor;
        nestedExecutor.setExecutionMode(executionMode);
        connect(&nestedExecutor, &SqlFileExecutor::schemaNeedsRefreshing, this, &SqlFileExecutor::schemaNeedsRefreshing);
        connect(&nestedExecutor, &SqlFileExecutor::execErrors, this, [this, &errors](const QList<QPair<QString, QString>>& nestedErrors, bool rolledBack)
        {
            errors += nestedErrors;
        });;
        nestedExecutor.execSqlFromFile(db, path, ignoreErrors, codec, false);
        return;
    }

    qDebug() << "Ignored unsupported dot command in SQL file:" << cmdLine;;
}

QStringList SqlFileExecutor::splitArgs(const QString& line)
{
    QStringList result;
    QString current;
    bool inQuotes = false;
    for (QChar c : line)
    {
        if (c == '"')
        {
            inQuotes = !inQuotes;
            continue;
        }

        if (c.isSpace() && !inQuotes)
        {
            if (!current.isEmpty())
            {
                result << current;
                current.clear();
            }
        }
        else
            current.append(c);
    }

    if (!current.isEmpty())
        result << current;

    return result;
}

void SqlFileExecutor::rollback()
{
    if (txName.isNull())
    {
        qCritical() << "No txName while calling SqlFileExecutor::rollback()";
        return;
    }

    db->rollback(txName);
    txName = QString();
}

bool SqlFileExecutor::commit()
{
    if (txName.isNull())
    {
        qCritical() << "No txName while calling SqlFileExecutor::commit()";
        return false;
    }

    bool res = db->commit(txName);
    txName = QString();
    return res;
}

SqlFileExecutor::ExecutionMode SqlFileExecutor::getExecutionMode() const
{
    return executionMode;
}

void SqlFileExecutor::setExecutionMode(ExecutionMode newExecutionMode)
{
    executionMode = newExecutionMode;
}
